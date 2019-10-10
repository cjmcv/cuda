#ifndef HCS_PROFILER_H_
#define HCS_PROFILER_H_

#include <iostream>

#include "util/internal_thread.hpp"
#include "executor.hpp"
#include "graph.hpp"

namespace hcs {

// TODO: 1. 时间测试，在内部调用，在demo中将不能使用timer。
class Profiler :public InternalThread {
public:
  Profiler(Executor *exec, Graph *graph) { exec_ = exec; graph_ = graph; }
  void Config(int mode, int ms) {
    mode_ = mode;
    interval_ms_ = ms;

    LogMessage::min_log_level_ = INFO;
  }
  ~Profiler() { 
    LogMessage::min_log_level_ = WARNING; 
    Timer::is_record_ = false;
  }

private:
  void ViewNode() {
    std::ostringstream stream;
    for (int i = 0; i < graph_->nodes().size(); i++) {
      Node *n = &(*(graph_->nodes()[i]));
      stream << "(" << n->name().c_str() << ":";
      stream << " " << n->num_cached_buf(0) << " ";
      for (int si = 1; si < n->num_successors(); si++) {
        stream << " " << n->num_cached_buf(si) << " ";
      }
      stream << "<" << n->num_empty_buf() << ">";
      stream << ", " << n->run_count() << ")";
    }
    LOG(INFO) << "Profiler->ViewNode: " << stream.str();
  }

  void ViewStatus() {
    bool flag = false;
    std::ostringstream stream;
    std::vector<Executor::Status*> list = exec_->status_list_;
    for (int i = 0; i < list.size(); i++) {
      if (list[i] == nullptr || list[i]->num_incomplete_out_nodes == 0) {
        continue;
      }
      else {
        flag = true;
        stream << "<" << i << ", " << list[i]->num_incomplete_out_nodes << ">";
      }
    }
    if (flag)
      LOG(INFO) << stream.str();
    else
      LOG(INFO) << "Profiler->ViewStatus: There's no active Status";
  }

  void ViewTimer() {
    Timer::is_record_ = true;
    std::ostringstream stream;
    for (int i = 0; i < exec_->timers_.size(); i++) {
      Timer *timer = exec_->timers_[i];
      stream << "(" << timer->node_name().c_str() << ": ";
      stream << "count-" << timer->count() << ", ";
      stream << "min-" << timer->min() << ", ";
      stream << "max-" << timer->max() << ", ";
      stream << "ave-" << timer->ave() << "). ";
    }
    LOG(INFO) << "Profiler->ViewTimer: " << stream.str();
  }

  void Entry() {
    while (!IsMustStop()) {
      // Note: "==" has a higher priority than "&".
      if ((mode_ & LOCK_RUN_TO_SERIAL) == LOCK_RUN_TO_SERIAL) {
        Timer::lock2serial_ = true; // TODO: 转移到其他地方，不放在timer
      }
      if ((mode_ & VIEW_NODE) == VIEW_NODE) {
        ViewNode();
      }
      if ((mode_ & VIEW_STATUS) == VIEW_STATUS) {
        ViewStatus();
      }
      if ((mode_ & VIEW_TIMER) == VIEW_TIMER) {
        ViewTimer();
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms_));
    }
  }

private:
  Executor *exec_;
  Graph *graph_;

  int mode_;
  int interval_ms_;
};

}  // namespace hcs.

#endif // HCS_PROFILER_H_