#ifndef HCS_COMMON_H_
#define HCS_COMMON_H_

namespace hcs {

enum ExecutorMode {
  SERIAL = 0,
  PARALLEL,
  PARALLEL_STREAMS
};

enum MemoryMode {
  ON_HOST = 0,
  ON_DEVICE
};

enum TypeFlag {
  FLOAT32 = 0,
  INT32 = 1,
  INT8 = 2,
  TYPES_NUM = 3  // Used to mark the total number of elements in TypeFlag.
};

enum ProfilerMode {
  LOCK_RUN_TO_SERIAL = 0x01,
  VIEW_NODE = 0x02,
  VIEW_STATUS = 0x04,
  VIEW_NODE_RUN_TIME = 0x08,
  VIEW_STATUS_RUN_TIME = 0x16,
};

// TODO: task_assistor_����ѡ�񲻸�ֵ
class TaskAssistor {
public:
  TaskAssistor() {}
  ~TaskAssistor() {
    if (streams_ != nullptr) {
      for (int i = 0; i < num_streams_; i++) {
        cudaStreamDestroy(streams_[i]);
      }
      free(streams_);
      streams_ = nullptr;
    }
  }
  
  // Thread local variables.
  inline int id() const {
    return thread_var().id;
  }
  struct ThreadVar {
    int id = -1;
  };
  inline ThreadVar &thread_var() const {
    thread_local ThreadVar thread_var;
    return thread_var;
  }

  // Initialization for GPU side. 
  // Create streams for each thread.
  void Init4GPU(int num_streams) {
    num_streams_ = num_streams;
    streams_ = (cudaStream_t *)malloc(num_streams_ * sizeof(cudaStream_t));
    for (int i = 0; i < num_streams_; i++) {
      cudaStreamCreate(&(streams_[i]));
      printf("<s-%d, %d>", i, streams_[i]);
    }
  }
  inline cudaStream_t stream() const { return streams_[id()]; }

private:
  int num_streams_;
  cudaStream_t *streams_;
};

}  // namespace hcs.

#endif // HCS_COMMON_H_