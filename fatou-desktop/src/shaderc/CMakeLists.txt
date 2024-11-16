
#add_library(fatou-shaderc SHARED)
#set_target_properties(fatou-shaderc
#    PROPERTIES
#    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>
#    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>
#)

#target_compile_definitions(fatou-shaderc PRIVATE FATOULIBRARY_EXPORTS=1)

file(GLOB SRC_FILES "api/*.cpp" "api/*.h" "include/*.h" "*.cpp" "*.h")
target_sources(fatou PRIVATE ${SRC_FILES})
#target_sources(fatou-shaderc PRIVATE "../gui/cef/database.cpp")

#target_precompile_headers(fatou-shaderc PRIVATE ${FATOU_SRC}/precompiled.h)
#target_link_libraries(fatou fatou-shaderc)
#target_include_directories(fatou PRIVATE ./include)

