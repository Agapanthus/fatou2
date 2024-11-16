#include "../include/fatou-shaderc.h"

#include <iostream>
#include <shaderc/shaderc.hpp>

#include "../../window/console.h"
/*
class GlobalThing {
    DebugConsole console;

  public:
    GlobalThing() { console.start(1024); }
};
//GlobalThing thing;

// Returns GLSL shader source text after preprocessing.
std::string preprocess_shader(const std::string &source_name,
                              shaderc_shader_kind kind,
                              const std::string &source) {
    shaderc::Compiler compiler;
    shaderc::CompileOptions options;

    // Like -DMY_DEFINE=1
    options.AddMacroDefinition("MY_DEFINE", "1");

    shaderc::PreprocessedSourceCompilationResult result =
        compiler.PreprocessGlsl(source, kind, source_name.c_str(), options);

    if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
        std::cerr << result.GetErrorMessage();
        return "";
    }

    return {result.cbegin(), result.cend()};
}

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
        options.SetOptimizationLevel(shaderc_optimization_level_size);

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
    options.AddMacroDefinition("MY_DEFINE", "1");
    if (optimize)
        options.SetOptimizationLevel(shaderc_optimization_level_size);

    shaderc::SpvCompilationResult module =
        compiler.CompileGlslToSpv(source, kind, source_name.c_str(), options);

    if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
        std::cerr << module.GetErrorMessage();
        return std::vector<uint32_t>();
    }

    return {module.cbegin(), module.cend()};
}
*/
int compileShader() {

    const char kShaderSource[] = "#version 310 es\n"
                                 "void main() { int x = MY_DEFINE; }\n";

    /*{ // Preprocessing
        auto preprocessed = preprocess_shader(
            "shader_src", shaderc_glsl_vertex_shader, kShaderSource);
        std::cout << "Compiled a vertex shader resulting in preprocessed text:"
                  << std::endl
                  << preprocessed << std::endl;
    }

    { // Compiling
        auto assembly = compile_file_to_assembly(
            "shader_src", shaderc_glsl_vertex_shader, kShaderSource);
        std::cout << "SPIR-V assembly:" << std::endl << assembly << std::endl;

        auto spirv = compile_file("shader_src", shaderc_glsl_vertex_shader,
                                  kShaderSource);
        std::cout << "Compiled to a binary module with " << spirv.size()
                  << " words." << std::endl;
    }*/
/*
    { // Compiling with optimizing
        auto assembly =
            compile_file_to_assembly("shader_src", shaderc_glsl_vertex_shader,
                                     kShaderSource,  true);
        std::cout << "Optimized SPIR-V assembly:" << std::endl
                  << assembly << std::endl;

        auto spirv = compile_file("shader_src", shaderc_glsl_vertex_shader,
                                  kShaderSource, true);
        std::cout << "Compiled to an optimized binary module with "
                  << spirv.size() << " words." << std::endl;
    }
*/
/*
    { // Error case
        const char kBadShaderSource[] =
            "#version 310 es\nint main() { int main_should_be_void; }\n";

        std::cout << std::endl << "Compiling a bad shader:" << std::endl;
        compile_file("bad_src", shaderc_glsl_vertex_shader, kBadShaderSource);
    }

    { // Compile using the C API.
        std::cout << "\n\nCompiling with the C API" << std::endl;

        // The first example has a compilation problem.  The second does not.
        const char source[2][80] = {"void main() {}",
                                    "#version 450\nvoid main() {}"};

        shaderc_compiler_t compiler = shaderc_compiler_initialize();
        for (int i = 0; i < 2; ++i) {
            std::cout << "  Source is:\n---\n" << source[i] << "\n---\n";
            shaderc_compilation_result_t result = shaderc_compile_into_spv(
                compiler, source[i], std::strlen(source[i]),
                shaderc_glsl_vertex_shader, "main.vert", "main", nullptr);
            auto status = shaderc_result_get_compilation_status(result);
            std::cout << "  Result code " << int(status) << std::endl;
            if (status != shaderc_compilation_status_success) {
                std::cout << "error: "
                          << shaderc_result_get_error_message(result)
                          << std::endl;
            }
            shaderc_result_release(result);
        }
        shaderc_compiler_release(compiler);
    }*/

    return 0;
}