cmake_minimum_required(VERSION 3.19 FATAL_ERROR)

project(fatou VERSION 0.1.0)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# enable testing
# include(CTest)
# enable_testing()

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Only generate Debug and Release configuration types.
set(CMAKE_CONFIGURATION_TYPES Debug Release)

# without the gui, compilation and startup-time are much faster and the binary is also tiny
set(WITH_GUI True)

# you can remove most of the submodules from the solution once you have built them once
set(BUILD_DEPS False)

if(WITH_GUI)
    add_compile_definitions(WITH_GUI)
endif()

set(THIRD_PARTY_DIR ${PROJECT_SOURCE_DIR}/third_party)

# ########################################
# Add files

# automatically update all file dependencies (without re-configuring CMake)
add_executable(fatou WIN32)

# you have to build "all targets" first, but using this as startup is faster afterwards!
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT fatou)

set(FATOU_SRC ${PROJECT_SOURCE_DIR}/fatou-desktop/src)

target_sources(fatou PRIVATE ${FATOU_SRC}/main.cpp ${FATOU_SRC}/precompiled.h)
target_precompile_headers(fatou PRIVATE ${FATOU_SRC}/precompiled.h)

target_include_directories(fatou PRIVATE ${FATOU_SRC})
add_subdirectory(${FATOU_SRC}/window)

# add_subdirectory(${FATOU_SRC}/gui)
if(WITH_GUI)
    add_subdirectory(${FATOU_SRC}/gui/cef)
endif()

add_subdirectory(${FATOU_SRC}/shaderc)

# add_subdirectory(${FATOU_SRC}/gui/resource_manager)

# for compatibility with cef
set_property(TARGET fatou PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
add_definitions(-DUNICODE -D_UNICODE)

# more optimization
set_target_properties(fatou PROPERTIES COMPILE_OPTIONS "$<$<CONFIG:Release>:/GL>;$<$<CONFIG:Release>:/Oi>;$<$<CONFIG:Release>:/Ot>")
set_target_properties(fatou PROPERTIES LINK_FLAGS_RELEASE "/LTCG")

if(WITH_GUI)
    # this adds an manifest to visual studio. This is necessary for CEF (especially high dpi enabled apps in windows 10) to work properly
    set(SHARED_EXE_MANIFEST ${FATOU_SRC}/gui/shared/resources/win/shared.exe.manifest)
    set(SHARED_COMPATIBILITY_MANIFEST ${FATOU_SRC}/gui/shared/resources/win/compatibility.manifest)
    add_custom_command(
        TARGET fatou
        POST_BUILD
        COMMAND "mt.exe" -nologo
        -manifest \"${SHARED_EXE_MANIFEST}\"
        \"${SHARED_COMPATIBILITY_MANIFEST}\"
        -outputresource:"${CMAKE_BINARY_DIR}/$<CONFIGURATION>/fatou.exe"\;\#1
        COMMENT "Adding manifest..."
    )
endif()

# target_sources(fatou PRIVATE ${FATOU_SRC}/gui/shared/resources/win/shared.rc)

# for get DPI
target_link_libraries(fatou "Shcore")

# ##############
# shaderc
if(BUILD_DEPS)
    # weird! This can be disabled for compilation, buf when using them all, shaderc will crash!
    # set(SPIRV_SKIP_EXECUTABLES ON CACHE BOOL "" FORCE)
    # set(SPIRV_SKIP_INSTALL ON CACHE BOOL "" FORCE)
    # set(SPIRV_HEADERS_SKIP_INSTALL ON CACHE BOOL "" FORCE)
    # set(SKIP_SPIRV_TOOLS_INSTALL ON CACHE BOOL "" FORCE)
    # SET(SPIRV_SKIP_TESTS ON CACHE BOOL "" FORCE)
    # SET(SHADERC_SKIP_EXAMPLES ON CACHE BOOL "")
    # SET(SHADERC_SKIP_INSTALL ON CACHE BOOL "")
    # SET(SHADERC_SKIP_TESTS ON CACHE BOOL "")
    # SET(SKIP_GLSLANG_INSTALL ON CACHE BOOL "")
    # SET(EFFCEE_BUILD_SAMPLES OFF CACHE BOOL "")
    # SET(EFFCEE_BUILD_TESTING OFF CACHE BOOL "")
    # SET(BUILD_TESTING OFF CACHE BOOL "")
    # SET(ENABLE_CTEST OFF CACHE BOOL "")
    find_package(Python3 COMPONENTS Interpreter Development)
    execute_process(
        COMMAND ${Python3_EXECUTABLE}
        ${THIRD_PARTY_DIR}/shaderc/utils/git-sync-deps
        WORKING_DIRECTORY ${THIRD_PARTY_DIR}/shaderc
        RESULT_VARIABLE EXECUTE_RV
    )
    message(STATUS ${EXECUTE_RV})

    add_subdirectory(${THIRD_PARTY_DIR}/shaderc)
endif()

# ShaderC
# set(shadercdir "C:/code/shaderc-2022.1")
target_include_directories(fatou PRIVATE ${shaderc_SOURCE_DIR}/libshaderc/include)
target_link_libraries(fatou ${CMAKE_BINARY_DIR}/third_party/shaderc/libshaderc/$<CONFIG>/shaderc_combined.lib)

# ########################################
# sqlite
add_subdirectory(${THIRD_PARTY_DIR}/sqlite-amalgamation)
target_include_directories(fatou PRIVATE ${THIRD_PARTY_DIR}/sqlite-amalgamation)
target_link_libraries(fatou SQLite3)

# ########################################
# blosc
set(BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(BUILD_BENCHMARKS OFF CACHE BOOL "" FORCE)
set(BUILD_FUZZERS OFF CACHE BOOL "" FORCE)
set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(${THIRD_PARTY_DIR}/c-blosc)
target_include_directories(fatou PRIVATE ${THIRD_PARTY_DIR}/c-blosc)
target_link_libraries(fatou blosc_static)

# ########################################
# brotli
SET(BUILD_TESTING OFF CACHE BOOL "")
add_subdirectory(${THIRD_PARTY_DIR}/brotli)
target_include_directories(fatou PRIVATE ${BROTLI_INCLUDE_DIRS})
target_link_libraries(fatou brotlicommon-static brotlidec-static brotlienc-static)

# ########################################
# Vulkan
find_package(Vulkan)

# Try extracting VulkanSDK path from ${Vulkan_INCLUDE_DIRS}
if(NOT ${Vulkan_INCLUDE_DIRS} STREQUAL "")
    set(VULKAN_PATH ${Vulkan_INCLUDE_DIRS})
    STRING(REGEX REPLACE "/Include" "" VULKAN_PATH ${VULKAN_PATH})
endif()

if(NOT Vulkan_FOUND)
    # CMake may fail to locate the libraries but could be able to
    # provide some path in Vulkan SDK include directory variable
    # 'Vulkan_INCLUDE_DIRS', try to extract path from this.
    message(STATUS "Failed to locate Vulkan SDK, retrying again...")

    if(EXISTS "${VULKAN_PATH}")
        message(STATUS "Successfully located the Vulkan SDK: ${VULKAN_PATH}")
    else()
        message(FATAL_ERROR "Error: Unable to locate Vulkan SDK.")
        return()
    endif()
endif()

message(STATUS "Using Vulkan: ${VULKAN_PATH}")

# Add any required preprocessor definitions here
add_definitions(-DVK_USE_PLATFORM_WIN32_KHR)

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    # Include Vulkan header files from Vulkan SDK
    # already part of vk-hpp
    #include_directories(AFTER ${VULKAN_PATH}/Include)

    # Link directory for vulkan-1
    link_directories(${VULKAN_PATH}/Bin;${VULKAN_PATH}/Lib;)
endif()

# Link the debug and release libraries to the project
target_link_libraries(fatou ${Vulkan_LIBRARY})

# ########################################
# vpp
#file(GLOB VPP_FILES ${THIRD_PARTY_DIR}/vpp/src/vpp/*.cpp)
#target_sources(fatou PRIVATE ${VPP_FILES})
#target_include_directories(fatou PRIVATE ${THIRD_PARTY_DIR}/vpp/include)

# ########################################
# vk-hpp
target_include_directories(fatou PRIVATE ${THIRD_PARTY_DIR}/Vulkan-Hpp/Vulkan-Headers/include)
target_include_directories(fatou PRIVATE ${THIRD_PARTY_DIR}/Vulkan-Hpp)

# ########################################
# VMA
set(VMA_STATIC_VULKAN_FUNCTIONS ON CACHE BOOL "")
set(VMA_DYNAMIC_VULKAN_FUNCTIONS OFF CACHE BOOL "")
add_subdirectory(${THIRD_PARTY_DIR}/VulkanMemoryAllocator)
target_link_libraries(fatou VulkanMemoryAllocator)
target_include_directories(fatou PRIVATE ${THIRD_PARTY_DIR}/VulkanMemoryAllocator/include)


# ########################################
# Boost
set(BOOST_ROOT C:/boost_1_79_0)
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.79.0 REQUIRED COMPONENTS filesystem regex)

if(Boost_FOUND)
    message("Using Boost: ${Boost_INCLUDE_DIRS}")
    target_include_directories(fatou PRIVATE ${Boost_INCLUDE_DIRS})
    target_link_libraries(fatou ${Boost_LIBRARIES})

# target_include_directories(fatou-shaderc PRIVATE ${Boost_INCLUDE_DIRS})
# target_link_libraries(fatou-shaderc ${Boost_LIBRARIES})
else()
    message(FATAL_ERROR "Error: Unable to locate Boost C++.")
    return()
endif()

# ##############
# glfw
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(${THIRD_PARTY_DIR}/Vulkan-Hpp/glfw)
target_link_libraries(fatou glfw)

# compatibility with cef
set_property(TARGET glfw PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

target_include_directories(fatou PRIVATE ${THIRD_PARTY_DIR}/glfw-3.3.7/include)

# ##############
# glm
add_subdirectory(${THIRD_PARTY_DIR}/glm)
target_link_libraries(fatou glm)

# to create information for vscode https://stackoverflow.com/a/50360945/6144727
add_definitions(-DCMAKE_EXPORT_COMPILE_COMMANDS=ON)

# ##############
# WebP
if(BUILD_DEPS)
    set(WEBP_BUILD_ANIM_UTILS OFF CACHE BOOL "")
    set(WEBP_BUILD_CWEBP OFF CACHE BOOL "")
    set(WEBP_BUILD_DWEBP OFF CACHE BOOL "")
    set(WEBP_BUILD_GIF2WEBP OFF CACHE BOOL "")
    set(WEBP_BUILD_IMG2WEBP OFF CACHE BOOL "")
    set(WEBP_BUILD_VWEBP OFF CACHE BOOL "")
    set(WEBP_BUILD_WEBPINFO OFF CACHE BOOL "")
    set(WEBP_BUILD_WEBPMUX OFF CACHE BOOL "")
    set(WEBP_BUILD_EXTRAS OFF CACHE BOOL "")

    # set(CMAKE_BUILD_TYPE "$<CONFIG>")
    add_subdirectory(${THIRD_PARTY_DIR}/libwebp)
endif()

target_include_directories(fatou PRIVATE ${THIRD_PARTY_DIR}/libwebp/src)
set(webplibdir ${CMAKE_BINARY_DIR}/third_party/libwebp/$<CONFIG>)
message(STATUS ${webplibdir})
target_link_libraries(fatou "${webplibdir}/webpdecoder.lib" "${webplibdir}/webpdemux.lib" "${webplibdir}/webp.lib" "shlwapi;ole32;windowscodecs")

# ########################################
# CEF (based on the "shared" example from the CEF repo)
if(WITH_GUI)
    message(STATUS "######### Loading cef as build target...")
    SET(WITH_EXAMPLES OFF CACHE BOOL "" FORCE)

    # override sandbox (will cause problems in debug builds)
    SET(USE_SANDBOX OFF CACHE BOOL "Enable or disable use of the sandbox." FORCE)

    add_subdirectory(${THIRD_PARTY_DIR}/cef-project)
endif()

if(WITH_GUI)
    message(STATUS "######### Loading cef as dependency...")

    # Specify the CEF binary distribution root directory.
    # set(CEF_ROOT "C:/code/cef-project/third_party/cef/cef_binary_96.0.16+g89c902b+chromium-96.0.4664.55_windows64")

    # Add the CEF binary distribution's cmake/ directory to the module path.
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CEF_ROOT}/cmake")

    # Load the CEF configuration (executes FindCEF.cmake).
    find_package(CEF REQUIRED)

    # add_subdirectory(${CEF_LIBCEF_DLL_WRAPPER_PATH} libcef_dll_wrapper)
    message(STATUS ${CEF_LIBCEF_DLL_WRAPPER_PATH})

    # compatibility with cef
    add_dependencies(fatou libcef_dll_wrapper)

    ADD_LOGICAL_TARGET("libcef_lib" "${CEF_LIB_DEBUG}" "${CEF_LIB_RELEASE}")
    target_link_libraries(fatou libcef_lib libcef_dll_wrapper ${CEF_STANDARD_LIBS})

    # Logical target used to link the cef_sandbox library.
    if(USE_SANDBOX)
        ADD_LOGICAL_TARGET("cef_sandbox_lib" "${CEF_SANDBOX_LIB_DEBUG}" "${CEF_SANDBOX_LIB_RELEASE}")
        target_link_libraries(fatou cef_sandbox_lib ${CEF_SANDBOX_STANDARD_LIBS})
    endif()

    include_directories(${CEF_INCLUDE_PATH})

    # Copy binary and resource files to the target output directory.
    COPY_FILES("fatou" "${CEF_BINARY_FILES}" "${CEF_BINARY_DIR}" "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
    COPY_FILES("fatou" "${CEF_RESOURCE_FILES}" "${CEF_RESOURCE_DIR}" "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
endif()

# ########################################
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
