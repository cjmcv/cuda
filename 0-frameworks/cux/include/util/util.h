/*!
* \brief Utility.
*/

#ifndef CUX_UTIL_H_
#define CUX_UTIL_H_

#include <iostream>
#include <chrono>

#include <cuda_runtime.h>
#include "device_launch_parameters.h"
#include <cublas_v2.h>

#include "util/half.h"

namespace cux {

////////////////
// Enumeration.
////////////////
enum Code {
  OK = 0,
  CANCELLED = 1,
  UNKNOWN = 2,
  INVALID_ARGUMENT = 3,
  DEADLINE_EXCEEDED = 4,
  NOT_FOUND = 5,
  ALREADY_EXISTS = 6,
  PERMISSION_DENIED = 7,
  UNAUTHENTICATED = 16,
  RESOURCE_EXHAUSTED = 8,
  FAILED_PRECONDITION = 9,
  ABORTED = 10,
  OUT_OF_RANGE = 11,
  UNIMPLEMENTED = 12,
  INTERNAL = 13,
  UNAVAILABLE = 14,
};

enum TypeFlag {
  kFloat32 = 0,
  kFloat16 = 1, 
  kInt32 = 2,
  kInt8 = 3,
};

enum OpRunMode {
  ON_HOST,
  ON_DEVICE
};

enum Shape {
  NUMBER,
  CHANNELS,
  HEIGHT,
  WIDTH
};

enum DataFetchMode {
  NO_PUSH,        // Get current data without pushing data across devices.
  PUSH,           // If you want to take data from CPU, and there's data in GPU, it pushes data from GPU to CPU.
  PUSH_IF_EMPTY   // Add push condition: the data to be fetched is empty.
};

////////////////
// Function.
////////////////
const char* CublasGetErrorString(cublasStatus_t error);

////////////////
// Class.
////////////////
class StrProcessor {
public:
  static std::string FetchSubStr(std::string &src_str, std::string start_str, std::string end_str) {
    int start_idx = src_str.find(start_str) + start_str.length();
    int end_idx = src_str.find(end_str, start_idx);
    return src_str.substr(start_idx, end_idx - start_idx);
  }
};

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

// Check for cublas error messages.
#define CUBLAS_CHECK(condition) \
  do { \
    cublasStatus_t status = condition; \
	  if(status != CUBLAS_STATUS_SUCCESS) {	\
		  fprintf(stderr, "CUBLAS_CHECK error in line %d of file %s : %s \n", \
              __LINE__, __FILE__, cux::CublasGetErrorString(status) ); \
		  exit(EXIT_FAILURE);	\
	  } \
  } while (0)

// Log
#define CUXLOG_ERR(format, ...) fprintf(stderr,"[ERROR]: "##format"\n", ##__VA_ARGS__); std::abort();
#define CUXLOG_WARN(format, ...) fprintf(stdout,"[WARN]: "##format"\n", ##__VA_ARGS__);
#define CUXLOG_INFO(format, ...) fprintf(stdout,"[INFO]: "##format"\n", ##__VA_ARGS__);
#define CUXLOG_COUT(format, ...) fprintf(stdout,"> "##format"\n", ##__VA_ARGS__);

//#define INSTANTIATE_CLASS(classname) \
//  char gInstantiationGuard##classname; \
//  template class classname<float>; \
//  template class classname<double>

#define INSTANTIATE_CLASS(classname) \
  char gInstantiationGuard##classname; \
  template class classname<float>

#define TYPE_SWITCH(type, DType, ...)               \
  switch (type) {                                   \
  case cux::TypeFlag::kFloat32:                     \
    {                                               \
      typedef float DType;                          \
      {__VA_ARGS__}                                 \
    }                                               \
    break;                                          \
  case cux::TypeFlag::kFloat16:                     \
    {                                               \
      typedef cux::half DType;                      \
      {__VA_ARGS__}                                 \
    }                                               \
    break;                                          \
  case cux::TypeFlag::kInt32:                       \
    {                                               \
      typedef int32_t DType;                        \
      {__VA_ARGS__}                                 \
    }                                               \
    break;                                          \
  case cux::TypeFlag::kInt8:                        \
    {                                               \
      typedef int8_t DType;                         \
      {__VA_ARGS__}                                 \
    }                                               \
    break;                                          \
  default:                                          \
    CUXLOG_ERR("Unknown type enum %d", type);       \
  }
////////////////
// Struct.
////////////////
struct Device {
  int id;
  cudaDeviceProp prop;
};

template<typename DType>
struct DataType;
template<>
struct DataType<float> {
  static const int kFlag = cux::TypeFlag::kFloat32;
};
template<>
struct DataType<cux::half> {
  static const int kFlag = cux::TypeFlag::kFloat16;
};
template<>
struct DataType<int32_t> {
  static const int kFlag = cux::TypeFlag::kInt32;
};
template<>
struct DataType<int8_t> {
  static const int kFlag = cux::TypeFlag::kInt8;
};

////////////////
// Class.
////////////////

// Timer for gpu.
class GpuTimer {
public:
  GpuTimer() {
    CUDA_CHECK(cudaEventCreate(&start_));
    CUDA_CHECK(cudaEventCreate(&stop_));
  }
  ~GpuTimer() {
    CUDA_CHECK(cudaEventDestroy(start_));
    CUDA_CHECK(cudaEventDestroy(stop_));
  }
  inline void Start() { CUDA_CHECK(cudaEventRecord(start_, NULL)); }
  inline void Stop() { CUDA_CHECK(cudaEventRecord(stop_, NULL)); }
  inline float MilliSeconds() {
    float elapsed;
    CUDA_CHECK(cudaEventSynchronize(stop_));
    CUDA_CHECK(cudaEventElapsedTime(&elapsed, start_, stop_));
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

// TODO: 2. ����CuxData����̬��̬�ڴ桢�첽���������롣��
//       4. �ڴ�أ������ȼ���
//       5. CPU���쳣����/�澯����/������
//       7. Layout�����Ч�ʷ�����
//       8. �����鿴����gpu�豸��deviceQuery - finish
//       9. cmake�����ɸѡ����
//       10. ����cmake������debug��cuda��demo���̵�debug�ĺ�ʱ���졣
//       11. CuxData��Ӱ뾫�ȣ�- Finish
//       11. gemm cublas�뾫��;
////
// TODO: 1. �㷨��cublas��Ӧ������ͳһ������ͳһ - Finish
//       2. �����ӷֳ������������ģ��Լ���һ���뼴�������ת�ã����Լ����ڴ�����������֡�
//
//       3. demo��1������������������Ԥȡ����2������������ϳɹ�ʽ������
//       4. Prefetcher, Ԥȡ����Ԥȡ���ݵ�GPU������IO��ʱ
//       5. BlockingQueue, ����������У����ڷ���Ԥȡ��Prefetcher������Ԥȡ������
//       6. InnerThread, �ڲ��̣߳�Ϊ���������滻���ݣ���ͬ������Ԥȡ��Prefetcher
//
//       7. ��demo�У����û��Զ���OP.
//       8. ʹ��ģ�����Op���������ͣ�������Ҫ���ÿһ��������дkernel��- Finish
//       9. DataType����ģ������ͻ�����͵ı������Ӷ��л��õ��õ�kernel
////
// TODO: 3rdparty: ���Ժ궨�帲�ǣ����ֶ�ѡ��ʹ��
//                 1.ʹ��gtest����ӵ�Ԫ����ģ��: ���ܲ���/��汾�˺��������֤/�쳣�����ж� - Finish
//                 2.ʹ��cublas����ӵ�Op����Ϊ���Ի�׼.-Finish
//                 https://nvlabs.github.io/cub/structcub_1_1_caching_device_allocator.html
//                 https://github.com/mratsim/Arraymancer/issues/112
//                 3.ʹ��cub����װ�Դ����ģ��.
//                 4.ʹ�����ݿ⣬��������ѯ���������ݱ���.
//                 5.python�ӿڷ�װ��ǰ������->����dll�������������Ľӿڣ�����python����Щ�ӿ�����װ��
//
} // cux.
#endif //CUX_UTIL_H_
