/*!
* \brief gemm: C(M, N) = A(M, K) * B(K, N). -> (height, width)
*/

#ifndef CUX_GEMM_HPP_
#define CUX_GEMM_HPP_

#include "util/util.h"
#include "operator.h"

namespace cux {

struct GEMMKernelParam {
  float alpha = 1.0;
  float beta = 0.0;

  GEMMKernelParam& operator=(const GEMMKernelParam& in) {
    alpha = in.alpha;
    beta = in.beta;
    return *this;
  }
};

class GEMM : public Operator {
public:
  GEMM(GEMMKernelParam &params) :kernel_params_(params), Operator(3, 3) {
    config_2d_.resize(gpu_kernel_cnt_);
  }
  static Operator *GEMM::Creator(std::string &params_str);

  void Help() const;
  int SetIoData(const std::vector< CuxData<float>* > &input,
                const std::vector< CuxData<float>* > &output);
  void RunOnHost();
  void RunOnDevice();

  std::string &GetHostKernelsInfo(int kernel_id);
  std::string &GetDeviceKernelsInfo(int kernel_id);

  void GEMMHost(const int kernel_id, 
                const int M, const int N,
                const int K, const float ALPHA,
                const float *A, const int lda,
                const float *B, const int ldb,
                const float beta,
                float *C, const int ldc);

  void GEMMDevice(const int kernel_id,
                  const int M, const int N, 
                  const int K, const float ALPHA,
                  const float *A, const int lda,
                  const float *B, const int ldb,
                  const float beta,
                  float *C, const int ldc);

  void PrepareLaunchConfig(int N, int M);

private:
  CuxData<float> *A_;
  CuxData<float> *B_;
  CuxData<float> *C_;

  GEMMKernelParam kernel_params_;
  std::vector<Config2D> config_2d_;
};
} // cux.

#endif //CUX_GEMM_HPP_