#pragma once

#include "../../drawable.hpp"
#include "../../shader.hpp"
#include "../../texture.hpp"
#include "../computables/marching_cubes.hpp"

// Particles will receive their vertex attributes after compute shader processes
// positions
class Particles : public Drawable<Particles> {
public:
    Particles(VertexArrayObject&& vao, VertexBufferObject&& vbo, GLuint amount) 
        : Drawable(std::move(vao)), 
          _vbo(std::move(vbo)),
          _shader( // Big yikes
              Shader::create(
                  ShaderInfo { "shaders/mvm.vert", ShaderType::VERTEX }, 
                  ShaderInfo { "shaders/mvm.frag", ShaderType::FRAGMENT })),
          _amount(amount),
          _marching_cubes_compute_stage1(MarchingCubesCompute::create(amount))
    {
    }

    static std::shared_ptr<Particles> create_impl(GLuint amount) {
        auto vao = VertexArrayObject();
        auto vbo = VertexBufferObject(VertexBufferType::ARRAY);

        vao.bind();
        vbo.bind();

        // Reserve data in buffer
        vbo.send_data_raw(nullptr, amount * sizeof(glm::vec3), StorageType::DYNAMIC);

        vbo.enable_attribute_pointer(0, 4, VertexDataType::FLOAT, 4, 0);
        
        vao.unbind();

        return std::make_shared<Particles>(std::move(vao), std::move(vbo), amount);
    }

    DrawType draw_impl(glm::mat4& view, glm::mat4& projection) const {
        _vao.bind();

        auto draw_type = DrawArrays {
            VertexPrimitive::POINTS,
            0,
            _amount
        };

        _shader.use();
        _shader.set_mat4("projection", projection);
        _shader.set_mat4("view", view);
        _shader.set_mat4("model", glm::translate(glm::mat4x4(1.0), glm::vec3(-5.0f, -5.0f, 20.0f)));

        return DrawType(draw_type);
    }

    void update(float sea_level)
    {
        _marching_cubes_compute_stage1->dispatch(sea_level);    

        glm::vec4 *data = _marching_cubes_compute_stage1->expose_buffer();

        // Send updated data to buffer
        _vbo.bind();
        _vbo.send_data_raw(data, _amount * sizeof(glm::vec4), StorageType::DYNAMIC);
        _vbo.unbind();

        _marching_cubes_compute_stage1->close_buffer();
    }

private:
    VertexBufferObject _vbo;
    Shader _shader;
    GLuint _amount;
    std::shared_ptr<MarchingCubesCompute> _marching_cubes_compute_stage1;
};