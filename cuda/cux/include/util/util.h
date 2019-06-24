/*!
* \brief Utility.
*/

#ifndef CUX_UTIL_HPP_
#define CUX_UTIL_HPP_

#include <iostream>
#include <cuda_runtime.h>
#include "device_launch_parameters.h"
#include <chrono>

namespace cux {

////////////////
// Enumeration.
////////////////
enum RunMode {
  ON_HOST,
  ON_DEVICE
};

enum CuxShape {
  NUMBER,
  CHANNELS,
  HEIGHT,
  WIDTH
};

// TODO: CPU���쳣����/�澯����
////////////////
// Macro.
////////////////

// Check for cuda error messages.
#define CUDA_CHECK(condition) \
  do { \
    cudaError_t error = condition; \
    if (error != cudaSuccess) { \
      fprintf(stderr, "CUDA_CHECK error in line %d of file %s : %s \n", \
              __LINE__, __FILE__, cudaGetErrorString(cudaGetLastError()) ); \
      exit(EXIT_FAILURE); \
    } \
  } while(0);

// Log
#define CUXLOG_ERR(format, ...) fprintf(stderr,"[ERROR]: "##format"\n", ##__VA_ARGS__);
#define CUXLOG_WARN(format, ...) fprintf(stdout,"[WARN]: "##format"\n", ##__VA_ARGS__);
#define CUXLOG_INFO(format, ...) fprintf(stdout,"[INFO]: "##format"\n", ##__VA_ARGS__);
#define CUXLOG_COUT(format, ...) fprintf(stdout,"> "##format"\n", ##__VA_ARGS__);

////////////////
// Class.
////////////////

// Timer for gpu.
class GpuTimer {
public:
  GpuTimer() {
    cudaEventCreate(&start_);
    cudaEventCreate(&stop_);
  }
  ~GpuTimer() {
    cudaEventDestroy(start_);
    cudaEventDestroy(stop_);
  }
  inline void Start() { cudaEventRecord(start_, NULL); }
  inline void Stop() { cudaEventRecord(stop_, NULL); }
  inline float MilliSeconds() {
    float elapsed;
    cudaEventSynchronize(stop_);
    cudaEventElapsedTime(&elapsed, start_, stop_);
    return elapsed;
  }

protected:
  cudaEvent_t start_;
  cudaEvent_t stop_;
};

// Timer for cpu.
class CpuTimer {
public:
  typedef std::chrono::high_resolution_clock clock;
  typedef std::chrono::nanoseconds ns;

  inline void Start() { start_time_ = clock::now(); }
  inline void Stop() { stop_time_ = clock::now(); }
  inline float NanoSeconds() {
    return (float)std::chrono::duration_cast<ns>(stop_time_ - start_time_).count();
  }

  // Returns the elapsed time in milliseconds.
  inline float MilliSeconds() { return NanoSeconds() / 1000000.f; }

  // Returns the elapsed time in microseconds.
  inline float MicroSeconds() { return NanoSeconds() / 1000.f; }

  // Returns the elapsed time in seconds.
  inline float Seconds() { return NanoSeconds() / 1000000000.f; }

protected:
  std::chrono::time_point<clock> start_time_;
  std::chrono::time_point<clock> stop_time_;
};

// 
class StrProcessor {
public:
  static std::string FetchSubStr(std::string &src_str, std::string start_str, std::string end_str) {
    int start_idx = src_str.find(start_str) + start_str.length();
    int end_idx = src_str.find(end_str, start_idx);
    return src_str.substr(start_idx, end_idx - start_idx);
  }
};

// TODO: 1. ���ܲ���ģ�飬��IO��kernel�� - Finish
//       2. ��Ϣ��ӡ / ��־���ģ��. - Finish - TODO: ����
//       3. �ڴ�� / �Դ�أ������ȼ���
//
// TODO: 1. Prefetcher, Ԥȡ����Ԥȡ���ݵ�GPU������IO��ʱ
//       2. BlockingQueue, ����������У����ڷ���Ԥȡ��Prefetcher������Ԥȡ������
//       3. InnerThread, �ڲ��̣߳�Ϊ���������滻���ݣ���ͬ������Ԥȡ��Prefetcher
//
// TODO: 3rdparty: ���Ժ궨�帲�ǣ����ֶ�ѡ��ʹ��
//                 1.ʹ��gtest����ӵ�Ԫ����ģ��: ���ܲ���/��汾�˺��������֤/�쳣�����ж�
//                 2.ʹ��cublas����ӵ�Op����Ϊ��ʱ��׼.
//                 3.ʹ��cub����װ�Դ����ģ��.
//                 4.ʹ�����ݿ⣬��������ѯ���������ݱ���.
//                 5.python�ӿڷ�װ?
//       
} // cux.
#endif //CUX_UTIL_HPP_