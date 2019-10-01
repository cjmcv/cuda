#ifndef HCS_EXECUTOR_H_
#define HCS_EXECUTOR_H_

#include <iostream>
#include <random>
#include <atomic>
#include <thread>
#include <cassert>
#include <future>
#include <map>

#include "graph.hpp"

namespace hcs {

// The executor class to run a graph.
class Executor {
  // Just for debugging.
  friend class Profiler;

  struct PerThread {
    bool is_worker = false;
    int worker_id = -1;
  };

  struct Promise {
    std::promise<void> promise;
  };
  struct Status { 
    int num_incomplete_out_nodes;
    Promise *p = nullptr;

    ~Status() {
      if (p != nullptr) {
        delete p;
        p = nullptr;
      }
    }
    void CopyTo(Status *s) {
      s->num_incomplete_out_nodes = this->num_incomplete_out_nodes;
      if (s->p == nullptr)
        s->p = new Promise;
    }
  };

public:
  std::string name_;

  // constructs the executor with N worker threads
  explicit Executor(unsigned N = std::thread::hardware_concurrency()) :
    graph_{ nullptr },
    run_count_{ 0 } {}

  ~Executor() {
    // Clear.
    if (status_list_.size() > 0) {
      for (int i = 0; i < status_list_.size(); i++) {
        delete status_list_[i];
      }
      status_list_.clear();
    }

    // Shut down the scheduler
    done_ = true;
    // Nodtify and join
    NotifyAll();
    for (auto& t : threads_) {
      t.join();
    }
  }

  void Bind(Graph *g);  
  // TODO: Match wait() with all task.
  std::future<void> Run();
  void NotifyAll();

private:

  void Spawn(std::vector<std::unique_ptr<Node>> &nodes);

  bool WaitCheckInputs(Node *node);

  void NotifySuccessors(Node *node);

  void Stop(int id);

private:
  Graph *graph_;
  std::vector<Status*> status_list_;
  int run_count_;

  std::vector<std::thread> threads_;

  std::atomic<size_t> num_actives_{ 0 };
  std::atomic<size_t> num_thieves_{ 0 };
  std::atomic<bool> done_{ 0 };

  std::mutex mutex_;
};

void Executor::Spawn(std::vector<std::unique_ptr<Node>> &nodes) {

  for (unsigned i = 0; i < nodes.size(); ++i) {
    Node *node = &(*nodes[i]);
    if (node->num_dependents() == 0) {
      continue;
    }
    printf("Info: spawn %s.\n", node->name().c_str());
    if (!(node->has_task())) {
      printf("Error: No task can be invoked in %s.\n", node->name().c_str());
      continue;
    }

    threads_.emplace_back([this, i, node]() -> void {

      while (!done_) {
        // Wait and check intputs from depends.
        bool is_pass = WaitCheckInputs(node);
        if (!is_pass) { break; }

        std::vector<Blob *> inputs;
        Blob *output = nullptr;

        { // Borrow input & Prepare output.
          std::unique_lock<std::mutex> locker(mutex_);
          node->BorrowInputs(inputs);
          node->PrepareOutput(&output);
        }

        node->Run(inputs, output);

        { // Recycle inputs & Push output.
          std::unique_lock<std::mutex> locker(mutex_);
          node->RecycleInputs(inputs);
          node->PushOutput(output);
        }

        NotifySuccessors(node);
      }
    });
  }
}

bool Executor::WaitCheckInputs(Node *node) {

  std::unique_lock<std::mutex> locker(node->mutex_);
  bool is_ready = false;
  while (!is_ready) {
    is_ready = true;
    for (int i = 0; i < node->num_dependents(); ++i) {
      if (node->IsDependsOutEmpty(i)) {
        is_ready = false;
        break;
      }
    }

    if (!is_ready) {
      node->cond_.wait(locker);
      if (done_) {
        locker.unlock();
        return false;
      }
      printf("Info: %s-waiting.\n", node->name().c_str());
    }
  }
  return true;
}

void Executor::NotifySuccessors(Node* node) {

  const auto num_successors = node->num_successors();
 
  for (int i = 0; i < num_successors; ++i) {
    // Check that all dependent nodes have cached output data.
    Node *successor = node->successor(i);
    bool is_ready = true;
    for (int j = 0; j < successor->num_dependents(); ++j) {
      if (successor->IsDependsOutEmpty(j)) {
        is_ready = false;
        break;
      }
    }
    // If the dependents are ready, wake up it.
    // Note: mutex_ in node is a member variable of node. 
    // So std::unique_lock<std::mutex> locker(mutex_) should only be use for 
    // std::condition_variable to prevent multiple locks from being invoked 
    // at the same time and causing unawakes.
    if (is_ready)
      successor->cond_.notify_one();
  }

  int status_id = (node->run_count() - 1) % status_list_.size(); 
  Status *status = status_list_[status_id];
  printf("Check %d.", status_id);
  // A node without any successor should check the termination of this run.
  if (num_successors == 0) {
    if (--(status->num_incomplete_out_nodes) == 0) {
      // It means that all of the output nodes have been completed.
      printf("Stop %d.", status_id);
      Stop(status_id);   // Finishing this Run.
    }
  }
}

void Executor::Stop(int id) {
  auto p{ std::move(status_list_[id]->p->promise) };

  // Delete Promise.
  delete status_list_[id]->p;
  // Recover.
  status_list_[id]->p = new Promise;
  status_list_[id]->num_incomplete_out_nodes = graph_->GetOutputNodes().size();

  // We set the promise in the end to response the std::future in Run().
  p.set_value();
}

void Executor::Bind(Graph *g) {

  for (int i = 0; i < g->buffer_queue_size(); i++) {
    Status *stat = new Status;
    stat->p = new Promise;
    stat->num_incomplete_out_nodes = g->GetOutputNodes().size();

    status_list_.push_back(stat);
  }

  graph_ = g;

  Spawn(graph_->nodes());
}

std::future<void> Executor::Run() {
  Status *stat = status_list_[run_count_ % status_list_.size()];
  run_count_++;

  std::vector<Node*> input_nodes = graph_->GetInputNodes();
  for (auto node : input_nodes) {
    for (size_t i = 0; i < node->num_successors(); ++i) {
      node->successor(i)->cond_.notify_one();
    }
  }

  std::future<void> future = stat->p->promise.get_future();
  return future;
}

void Executor::NotifyAll() {
  for (auto &node : graph_->nodes()) {
    node->cond_.notify_all();
  }
}

}  // end of namespace hcs
#endif // HCS_EXECUTOR_H_