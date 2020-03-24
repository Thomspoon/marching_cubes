#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <type_traits>
#include <utility>
#include <filesystem>
namespace fs = std::filesystem;
#include "glad/glad.h"
#include "glm/mat4x4.hpp"

enum ShaderType {
    VERTEX   = GL_VERTEX_SHADER,
    FRAGMENT = GL_FRAGMENT_SHADER,
    COMPUTE  = GL_COMPUTE_SHADER,
    NUM_SHADER_TYPES = 3
};

struct ShaderInfo {
    const char*       name;
    ShaderType shader_type;
};

class Shader {
public:
    template<bool...> struct bool_pack;
    template<bool... bs> 
    using all_equal = std::is_same<bool_pack<bs..., true>, bool_pack<true, bs...>>;


    template<typename ...Args>
    static Shader create(Args&&... info) {
        static_assert(
            ((std::is_same<Args, ShaderInfo>::value) && ... && true), 
            "Only ShaderInfo is allows as a parameter to Shader::create"
        );

        // Initialize all shaders passed in
        uint8_t initialized_count = 0;
        GLuint shader_array[NUM_SHADER_TYPES] = {
            Shader::create_internal(info, initialized_count)...,
        };

        // Create program and attach shader programs
        auto program = GL_CHECK(glCreateProgram());
        for(auto index = 0u; index < initialized_count; index++)
        {
            GL_CHECK(glAttachShader(program, shader_array[index]));
        }

        // Link program and delete shader program memory
        Shader::link_program(program);
        for(auto index = 0u; index < initialized_count; index++)
        {
            GL_CHECK(glDeleteShader(shader_array[index]));
        }

        return Shader(program);
    }



    

    static GLuint create_internal(ShaderInfo& info, uint8_t& initialized_count)
    {
        std::string shader_source;
        std::ifstream shader_file;
        std::stringstream shader_stream;

        std::cout << "Shader: " << info.name << '\n';
        
        try {
            // ensure ifstream objects can throw exceptions:
            shader_file.exceptions (std::ifstream::failbit | std::ifstream::badbit);

            // open files
            shader_file.open(info.name);

            // read file's buffer contents into streams
            shader_stream << shader_file.rdbuf();

            // close file handlers
            shader_file.close();

            // convert stream into string
            shader_source = shader_stream.str();
        } catch (std::ifstream::failure& e) {
            throw std::runtime_error(std::string("Failed to compile shader: ") + std::string(info.name));
        }
        
        auto shader = Shader::compile_shader(shader_source.c_str(), info.shader_type);

        initialized_count++;

        return shader;
    }

    ~Shader() 
    {
        std::cout << "Shader destroyed!" << std::endl;
    }

    void use() const;

    void set_bool(const char *name, bool value) const;
    void set_int(const char *name, int value) const;
    void set_float(const char *name, float value) const;
    void set_vec2(const char *name, const glm::vec2 &value) const;
    void set_vec3(const char *name, const glm::vec3 &value) const;
    void set_vec4(const char *name, const glm::vec4 &value) const;
    void set_mat2(const char *name, const glm::mat2 &mat) const;
    void set_mat3(const char *name, const glm::mat3 &mat) const;
    void set_mat4(const char *name, const glm::mat4 &mat) const;


protected:
    Shader(GLuint);

    static GLuint compile_shader(const char *shader, GLenum shader_type);
    static GLuint link_program(GLuint program);

    GLuint m_program;

};