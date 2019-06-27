/*!
* \brief Dot Product.
*/

#ifndef CUX_DOT_PRODUCT_HPP_
#define CUX_DOT_PRODUCT_HPP_

#include "util/util.h"
#include "operator.h"

namespace cux {

// TODO: 1. 使用模板控制数据类型，可能需要针对每一种类型重写kernel.
class VectorDotProduct : public Operator {
public:
  VectorDotProduct() :cpu_kernel_cnt_(1), gpu_kernel_cnt_(3) {}
  static Operator *VectorDotProduct::Creator(std::string &params_str);
  
  void Help() const;
  int SetIoData(const std::vector< CuxData<float>* > &input,
                const std::vector< CuxData<float>* > &output);
  void RunOnHost();
  void RunOnDevice();

  void VectorDotProductDevice(const int kernel_id, const int blocks_per_grid,
                              const int threads_per_block, const float *vec_a,
                              const float *vec_b, const int len, float &res);

private:
  CuxData<float> *in_a_;
  CuxData<float> *in_b_;
  CuxData<float> *out_;

  int cpu_kernel_cnt_;
  int gpu_kernel_cnt_;
};
} // cux.

#endif //CUX_DOT_PRODUCT_HPP_