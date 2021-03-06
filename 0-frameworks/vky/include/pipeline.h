#ifndef VKY_PIPELINE_H_
#define VKY_PIPELINE_H_

#include <iostream>
#include <vulkan/vulkan.hpp>

#include "data_type.h"
#include "device.h"

namespace vky {

class Pipeline {

public:
  Pipeline() {
    num_descriptors_ = 0;
    push_constant_count_ = 0;
    local_shader_module_ = nullptr;
    descriptor_set_layout_ = nullptr;
    pipeline_layout_ = nullptr;
    pipeline_ = nullptr;
  }

  vk::PipelineLayout pipeline_layout() const { return pipeline_layout_; }
  vk::Pipeline pipeline() const { return pipeline_; }

  vk::DescriptorSet descriptor_set() const { return descriptor_set_; }

  int Initialize(const vk::Device device, 
                 const vk::ShaderModule shader, 
                 const uint32_t buffer_count,
                 const uint32_t push_constant_count,
                 const uint32_t *local_size);
  void UnInitialize();

  int CreateDescriptorPool();
  int DestroyDescriptorPool();

  int AllocateDescriptorSet();
  void FreeDescriptorSet();

  int UpdateDescriptorSet(const std::vector<vky::BufferMemory *> &buffer_memorys);

private:
  int CreateDescriptorsetLayout();
  void DestroyDescriptorsetLayout();
  int CreatePipelineLayout();
  void DestroyPipelineLayout();
  // Create compute pipeline consisting of a single stage with compute shader.
  // Specialization constants specialized here.
  int CreatePipeline(const vk::ShaderModule shader, const uint32_t *local_size);
  void DestroyPipeline();

private:
  vk::Device device_;

  vk::ShaderModule local_shader_module_;
  vk::DescriptorSetLayout descriptor_set_layout_; // channel++ definition of the shader binding interface
  vk::DescriptorPool descriptor_pool_;  // descriptors pool
  vk::DescriptorSet descriptor_set_;

  vk::PipelineLayout pipeline_layout_;
  vk::PipelineCache pipe_cache_;
  vk::Pipeline pipeline_;

  int num_descriptors_;
  int push_constant_count_;
}; // class Pipeline

} // namespace vky

#endif  // VKY_PIPELINE_H_