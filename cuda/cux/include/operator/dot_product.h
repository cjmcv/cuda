/*!
* \brief Dot Product.
*/

#ifndef CUX_DOT_PRODUCT_HPP_
#define CUX_DOT_PRODUCT_HPP_

#include "util.h"
#include "operator.h"

namespace cux {

// TODO: 1. ʹ��ģ������������ͣ�������Ҫ���ÿһ��������дkernel.
class VectorDotProduct : public Operator {
public:
  VectorDotProduct() {}

  void Help() const;

  int SetIoParams(const std::vector< CuxData<float>* > &input,
                  const std::vector< CuxData<float>* > &output,
                  const OpParam *params);
  void RunOnHost();
  void RunOnDevice();

private:
  CuxData<float> *in_a_;
  CuxData<float> *in_b_;
  CuxData<float> *out_;
};
} // cux.

#endif //CUX_DOT_PRODUCT_HPP_