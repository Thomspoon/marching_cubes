#pragma once

#include "../../drawable.hpp"
#include "glm/glm.hpp"

namespace {
    static float vertex_attributes[] = {
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f,
    };
}

class Cube : public Drawable<Cube> {
public:
    explicit Cube(VertexArrayObject&& t_vao, VertexBufferObject&& t_vbo) 
    : Drawable(std::move(t_vao)),
      vbo(std::move(t_vbo)),
      _shader( // Big yikes
          Shader::create(
              ShaderInfo { "shaders/mvm.vert", ShaderType::VERTEX }, 
              ShaderInfo { "shaders/mvm.frag", ShaderType::FRAGMENT })),
      _position(glm::vec3(0.0f, 0.0f, 30.0f)) {}

    static std::shared_ptr<Cube> create_impl() {
        auto cube_vao = VertexArrayObject();
        auto cube_vbo = VertexBufferObject(VertexBufferType::ARRAY);

        cube_vao.bind();
        cube_vbo.bind();

        cube_vbo.send_data(vertex_attributes, StorageType::STATIC);

        cube_vbo.enable_attribute_pointer(0, 3, VertexDataType::FLOAT, 3, 0);
        
        cube_vao.unbind();

        return std::make_shared<Cube>(std::move(cube_vao), std::move(cube_vbo));
    }

    DrawType draw_impl(glm::mat4& view, glm::mat4& projection) const {
        _vao.bind();

        auto draw_type = DrawArrays {
            VertexPrimitive::TRIANGLES,
            0,
            36
        };

        _shader.use();
        _shader.set_vec3("object_color", glm::vec3(1.0f, 1.0f, 1.0f));
        _shader.set_mat4("projection", projection);
        _shader.set_mat4("view", view);
        _shader.set_mat4("model", glm::translate(glm::mat4x4(1.0), _position));

        return DrawType(draw_type);
    }

    void update(glm::vec3 new_position) {
        _position = new_position;
    }

private:
    VertexBufferObject vbo;  

    Shader _shader;
    glm::vec3 _position;  
};