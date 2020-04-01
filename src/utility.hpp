#pragma once

#define __DEBUG__

#include <glad/glad.h>

// This is a convenience define that will check for errors after each hidden
// opengl call, and if API_DUMP is defined, it will also log gl calls 
// to stdout
#define API_DUMP 0
#ifdef __DEBUG__
#define GL_CHECK(func) func;                                                \
if(API_DUMP) {                                                              \
    std::cout << #func << std::endl;                                        \
}                                                                           \
if(GLenum err = glGetError()) {                                             \
    std::string err_string = "GL ERROR "                                    \
        + std::to_string(err)                                               \
        + " at line "                                                       \
        + std::to_string(__LINE__)                                          \
        + " in " #func " at "                                               \
        + std::string(__PRETTY_FUNCTION__);                                 \
    throw std::runtime_error(err_string);                                   \
}
#else
#define GL_CHECK(var) var;
#endif