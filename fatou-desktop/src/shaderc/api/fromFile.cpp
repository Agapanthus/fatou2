#include "../include/fatou-shaderc.h"

#include <iostream>
#include <shaderc/shaderc.hpp>
#include <fstream>

#include "shaderCache.h"

// Compiles a shader to SPIR-V assembly. Returns the assembly text
// as a string.
std::string compile_file_to_assembly(const std::string &source_name,
                                     shaderc_shader_kind kind,
                                     const std::string &source,
                                     bool optimize = false) {
    shaderc::Compiler compiler;
    shaderc::CompileOptions options;

    // Like -DMY_DEFINE=1
    options.AddMacroDefinition("MY_DEFINE", "1");
    if (optimize)
        options.SetOptimizationLevel(shaderc_optimization_level_performance);

    shaderc::AssemblyCompilationResult result =
        compiler.CompileGlslToSpvAssembly(source, kind, source_name.c_str(),
                                          options);

    if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
        std::cerr << result.GetErrorMessage();
        return "";
    }

    return {result.cbegin(), result.cend()};
}

// Compiles a shader to a SPIR-V binary. Returns the binary as
// a vector of 32-bit words.
std::vector<uint32_t> compile_file(const std::string &source_name,
                                   shaderc_shader_kind kind,
                                   const std::string &source,
                                   bool optimize = false) {
    shaderc::Compiler compiler;
    shaderc::CompileOptions options;

    // Like -DMY_DEFINE=1
    //options.AddMacroDefinition("MY_DEFINE", "1");
    if (optimize)
        options.SetOptimizationLevel(shaderc_optimization_level_performance);

    /*
    string source;
    std::ifstream file(
        "C:\\code\\clouds\\public\\shaders\\playground\\rainbowTriangle.vert",
        std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t)file.tellg();
    if (fileSize % sizeof(char) != 0) {
        throw runtime_error("fail has wrong size!");
    }
    vector<char> buffer(fileSize / sizeof(char));
    file.seekg(0);
    file.read((char *)buffer.data(), fileSize);
    file.close();

    source.assign(buffer.begin(), buffer.end());

    
    source = "#version 310 es\n"
             "void main() { int x = MY_DEFINE; }\n";
    
    source = "#version 450\n"
             "layout(location = 0) in vec2 inPosition;\n"
             "layout(location = 1) in vec3 inColor;\n"
             "layout(location = 2) in vec2 inTexCoord;\n"
             "layout(location = 0) out vec3 fragColor;\n"
             "layout(location = 1) out vec2 fragTexCoord;\n"
             "layout(binding = 0) uniform UniformBufferObject {\n"
             "    mat4 model;\n"
             "    mat4 view;\n"
             "    mat4 proj;\n"
             "} ubo;\n"
             "void main() {\n"
             "    gl_Position = ubo.proj * ubo.view * ubo.model *
    vec4(inPosition, 0.0, 1.0);\n" "    fragColor = inColor;\n\n" "
    fragTexCoord = inTexCoord;"
             "}\n";
    

    std::cout << source << std::endl;*/
    shaderc::SpvCompilationResult module =
        compiler.CompileGlslToSpv(source, kind, source_name.c_str(), options);

    if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
        const string str = module.GetErrorMessage();
        std::cerr << str;
        return std::vector<uint32_t>();
    }

    return {module.cbegin(), module.cend()};
}

void *copyVectorToArray(const vector<uint32_t> &vec, size_t &len) {
    len = vec.size() * sizeof(uint32_t);
    uint32_t *arr = new uint32_t[vec.size()];
    std::copy(vec.begin(), vec.end(), arr);
    return arr;
}

void *copyVectorToArray(const vector<uint8_t> &vec, size_t &len) {
    len = vec.size() * sizeof(uint8_t);
    uint8_t *arr = new uint8_t[vec.size()];
    std::copy(vec.begin(), vec.end(), arr);
    return arr;
}

void *compileShaderFromFile(const path &path, bool isVertex, size_t &len) {

    auto spv = cache.getCompiled(path);
    if (spv.has_value()) {
        return copyVectorToArray(spv.value(), len);
    }

    const string kShaderSource = cache.getSource(path);

    { // Compiling with optimizing
        /*auto assembly =
            compile_file_to_assembly("shader_src",
                                     isVertex ? shaderc_glsl_vertex_shader
                                              : shaderc_glsl_fragment_shader,
                                     kShaderSource, true);
        std::cout << "Optimized SPIR-V assembly:" << std::endl
                  << assembly << std::endl;

        if (!assembly.length()) {
            return nullptr;
        }*/

        auto spirv = compile_file("shader_src",
                                  isVertex ? shaderc_glsl_vertex_shader
                                           : shaderc_glsl_fragment_shader,
                                  kShaderSource, true);
        std::cout << "Compiled to an optimized binary module with "
                  << spirv.size() << " words." << std::endl;

        if (!spirv.size()) {
            return nullptr;
        }

        cache.store(path, spirv);
        return copyVectorToArray(spirv, len);
    }
}