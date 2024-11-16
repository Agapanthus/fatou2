#pragma once

#include <vector>
#include <string>
#include "logicalDevice.h"

namespace ShaderType {
enum ShaderType { VERTEX, FRAGMENT };
}

class Shader : private boost::noncopyable {
  public:
    Shader(shared_ptr<LogicalDevice> device, const path &path,
           const ShaderType::ShaderType type);
    ~Shader();
    vk::PipelineShaderStageCreateInfo getInfo() const;

  private:
    void createShaderModule(const vector<char> &code);
    vk::raii::ShaderModule shaderModule;
    shared_ptr<LogicalDevice> device;
    const ShaderType::ShaderType type;
};
