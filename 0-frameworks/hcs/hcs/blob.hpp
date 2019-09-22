#ifndef HCS_BLOB_H_
#define HCS_BLOB_H_

#include <iostream>
#include <vector>
#include <cuda_runtime.h>
#include "device_launch_parameters.h"

#include "common.hpp"
#include "util/util.hpp"

namespace hcs {

class Blob {
public:
  Blob() :is_created_(false), 
    data_(nullptr), 
    object_id_(-1), 
    num_element_(0),
    mode_(-1), 
    type_(-1) {
    shape_.clear();
  }
  ~Blob() { Release(); }

  bool Create(int num, int channel, int height, int width, int mode, int type);
  void Release();

  bool CopyTo(Blob *to);
  bool CloneTo(Blob *to);
  bool SyncParams(int num, int channel, int height, int width, int mode, int type);

public:
  void *data_;
  int object_id_;
  int num_element_;

private:
  bool is_created_;

  int mode_;
  int type_;

  std::vector<int> shape_;
};

bool Blob::Create(int num, int channel, int height, int width, int mode, int type) {
  object_id_ = -1;

  shape_.clear();
  shape_.push_back(num);
  shape_.push_back(channel);
  shape_.push_back(height);
  shape_.push_back(width);

  num_element_ = num * channel * height * width;

  mode_ = mode;
  type_ = type;

  if (num_element_ <= 0) { return -1; }

  if (mode_ == ON_HOST) {
    TYPE_SWITCH(type_, T, data_ = new T[num_element_];);
  }
  else {
    TYPE_SWITCH(type_, T,
      CUDA_CHECK(cudaMalloc(&data_, sizeof(T) * num_element_));
    );
  }

  is_created_ = true;
  return true;
}

void Blob::Release() {
  if (data_ != nullptr) {
    if (mode_ == ON_HOST) {
      delete[]data_;
    }
    else {
      CUDA_CHECK(cudaFree(data_));
    }
    data_ = nullptr;
  }
  is_created_ = false;
}

bool Blob::CopyTo(Blob *to) {
  if (mode_ != to->mode_) {
    printf("Error in CopyTo-> !mode_ != to->mode_.\n");
    return false;
  }

  if (mode_ == ON_HOST) {
    TYPE_SWITCH(type_, T,
      memcpy(to->data_, data_, sizeof(T) * num_element_);
    );
  }
  else {
    TYPE_SWITCH(type_, T,
      cudaMemcpy(to->data_, data_, sizeof(T) * num_element_, cudaMemcpyDeviceToDevice);
    );
  }
  return true;
}

bool Blob::SyncParams(int num, int channel, int height, int width, int mode, int type) {
  // Check dimension.
  if (is_created_ == false) {
    return Create(num, channel, height, width, mode, type);
  }

  int num_element = num * channel * height * width;
  if (mode_ != mode || type_ != type || num_element_ != num_element) {
    Release();
    return Create(num, channel, height, width, mode, type);
  }

  shape_.clear();
  shape_.push_back(num);
  shape_.push_back(channel);
  shape_.push_back(height);
  shape_.push_back(width);
}

bool Blob::CloneTo(Blob *to) {
  if (!is_created_) {
    printf("Error -> !is_created_.\n");
    return false;
  }
  
  to->SyncParams(shape_[0], shape_[1], shape_[2], shape_[3], mode_, type_);

  CopyTo(to);
  // Pass object id.
  to->object_id_ = object_id_;
  return true;
}

}  // namespace hcs.

#endif // HCS_BLOB_H_