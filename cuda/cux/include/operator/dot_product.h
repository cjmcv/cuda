/*!
* \brief Dot Product.
*/

#ifndef CUX_DOT_PRODUCT_HPP_
#define CUX_DOT_PRODUCT_HPP_

#include "util/util.h"
#include "operator.h"

namespace cux {

// TODO: 1. ʹ��ģ������������ͣ�������Ҫ���ÿһ��������дkernel.
class VectorDotProduct : public Operator {
public:
  VectorDotProduct() {}
  static Operator *VectorDotProduct::Creator(std::string &params_str);
  
  void Help() const;
  int SetIoData(const std::vector< CuxData<float>* > &input,
                const std::vector< CuxData<float>* > &output);
  void RunOnHost();
  void RunOnDevice();

private:
  CuxData<float> *in_a_;
  CuxData<float> *in_b_;
  CuxData<float> *out_;
};
} // cux.

#endif //CUX_DOT_PRODUCT_HPP_