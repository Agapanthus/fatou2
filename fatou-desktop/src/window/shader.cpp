
#include "shader.h"
#include <fstream>

#include "../shaderc/include/fatou-shaderc.h"

vector<char> vectorFromPointer(const uint8_t *mem, size_t len) {
    return vector<char>(mem, mem + len);
}

Shader::Shader(shared_ptr<LogicalDevice> device, const path &path,
               const ShaderType::ShaderType type)
    : device(device), type(type), shaderModule(nullptr) {
    size_t len;
    const void *mem =
        compileShaderFromFile(path, type == ShaderType::VERTEX, len);
    if (!mem)
        throw runtime_error("shader compilation failed");
    createShaderModule(vectorFromPointer((const uint8_t *)mem, len));
}

vk::PipelineShaderStageCreateInfo Shader::getInfo() const {
    vk::PipelineShaderStageCreateInfo shaderStageInfo{};
    shaderStageInfo.sType = vk::StructureType::ePipelineShaderStageCreateInfo;
    shaderStageInfo.module = *shaderModule;
    shaderStageInfo.pName = "main";
    // It allows you to specify values for shader constants. You can use a
    // single shader module where its behavior can be configured at pipeline
    // creation by specifying different values for the constants used in it.
    // This is more efficient than configuring the shader using variables at
    // render time, because the compiler can do optimizations like
    // eliminating if statements that depend on these values. If you don't
    // have any constants like that, then you can set the member to nullptr
    shaderStageInfo.pSpecializationInfo = nullptr;

    switch (type) {
    case ShaderType::VERTEX: {
        shaderStageInfo.stage = vk::ShaderStageFlagBits::eVertex;
    } break;
    case ShaderType::FRAGMENT: {
        shaderStageInfo.stage = vk::ShaderStageFlagBits::eFragment;
    } break;
    default:
        throw std::runtime_error("unknown shader type");
    }

    return shaderStageInfo;
}

Shader::~Shader() {
    //vkDestroyShaderModule(device->handle(), shaderModule, nullptr);
}

void Shader::createShaderModule(const vector<char> &code) {
    vk::ShaderModuleCreateInfo createInfo{};
    createInfo.sType = vk::StructureType::eShaderModuleCreateInfo;
    createInfo.codeSize = code.size();

    // When you perform a cast like this, you also need to ensure that the
    // data satisfies the alignment requirements of uint32_t. Lucky for us,
    // the data is stored in an vector where the default allocator already
    // ensures that the data satisfies the worst case alignment
    // requirements.
    createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

    shaderModule = device->device.createShaderModule(createInfo);

    // Shader modules are just a thin wrapper around the shader bytecode
    // that we've previously loaded from a file and the functions defined in
    // it.
}