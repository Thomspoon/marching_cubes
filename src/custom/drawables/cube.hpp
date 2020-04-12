#pragma once

#include "../../drawable.hpp"
#include "glm/glm.hpp"

namespace {
    static float vertex_attributes[] = {
        -0.5f, -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, -0.5f, 0.0f,
         0.5f,  0.5f, -0.5f, 0.0f,
         0.5f,  0.5f, -0.5f, 0.0f,
        -0.5f,  0.5f, -0.5f, 0.0f,
        -0.5f, -0.5f, -0.5f, 0.0f,
        -0.5f, -0.5f,  0.5f, 0.0f,
         0.5f, -0.5f,  0.5f, 0.0f,
         0.5f,  0.5f,  0.5f, 0.0f,
         0.5f,  0.5f,  0.5f, 0.0f,
        -0.5f,  0.5f,  0.5f, 0.0f,
        -0.5f, -0.5f,  0.5f, 0.0f,
        -0.5f,  0.5f,  0.5f, 0.0f,
        -0.5f,  0.5f, -0.5f, 0.0f,
        -0.5f, -0.5f, -0.5f, 0.0f,
        -0.5f, -0.5f, -0.5f, 0.0f,
        -0.5f, -0.5f,  0.5f, 0.0f,
        -0.5f,  0.5f,  0.5f, 0.0f,
         0.5f,  0.5f,  0.5f, 0.0f,
         0.5f,  0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f,  0.5f, 0.0f,
         0.5f,  0.5f,  0.5f, 0.0f,
        -0.5f, -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f,  0.5f, 0.0f,
         0.5f, -0.5f,  0.5f, 0.0f,
        -0.5f, -0.5f,  0.5f, 0.0f,
        -0.5f, -0.5f, -0.5f, 0.0f,
        -0.5f,  0.5f, -0.5f, 0.0f,
         0.5f,  0.5f, -0.5f, 0.0f,
         0.5f,  0.5f,  0.5f, 0.0f,
         0.5f,  0.5f,  0.5f, 0.0f,
        -0.5f,  0.5f,  0.5f, 0.0f,
        -0.5f,  0.5f, -0.5f, 0.0f
    };
}

class Cube : public Drawable<Cube> {
public:
    explicit Cube(
        VertexArrayObject&& t_vao,
        VertexBufferObject&& t_vbo,
        glm::vec3 position
    ) : _vao(std::move(t_vao)),
        _vbo(std::move(t_vbo)),
        _shader(
            Shader::create(
                ShaderInfo { "shaders/mvm.vert", ShaderType::VERTEX }, 
                ShaderInfo { "shaders/mvm.frag", ShaderType::FRAGMENT })),
        _position(position) {}

    static Cube create(glm::vec3 position) {
        auto cube_vao = VertexArrayObject();
        auto cube_vbo = VertexBufferObject(VertexBufferType::ARRAY);

        cube_vao.bind();
        cube_vbo.bind();

        cube_vbo.send_data(vertex_attributes, StorageType::STATIC);

        cube_vbo.enable_attribute_pointer(0, 4, VertexDataType::FLOAT, 4, 0);
        
        cube_vao.unbind();

        return Cube(
            std::move(cube_vao),
            std::move(cube_vbo),
            position
        );
    }

    void draw(glm::mat4& view, glm::mat4& projection) const {
        _vao.bind();

        _shader.use();
        _shader.set_vec3("object_color", glm::vec3(1.0f, 1.0f, 1.0f));
        _shader.set_mat4("projection", projection);
        _shader.set_mat4("view", view);
        _shader.set_mat4("model", glm::translate(glm::mat4x4(1.0), _position));
        glDrawArrays(GL_TRIANGLES, 0, 36);
        _vao.unbind();
    }

    void update(glm::vec3 new_position) {
        _position = new_position;
    }

    glm::vec3 get_position()
    {
        return _position;
    }

private:
    VertexArrayObject _vao;
    VertexBufferObject _vbo;  

    Shader _shader;
    glm::vec3 _position;  
};