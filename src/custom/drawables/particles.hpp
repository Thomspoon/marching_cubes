#pragma once

#include "../../drawable.hpp"
#include "../../shader.hpp"
#include "../../texture.hpp"
#include "../computables/marching_cubes.hpp"

// Particles will receive their vertex attributes after compute shader processes
// positions
class Particles : public Drawable<Particles> {
public:
    Particles(
        VertexArrayObject&& vao, 
        VertexBufferObject&& vbo, 
        VertexArrayObject&& vao_triangles, 
        VertexBufferObject&& vbo_triangles, 
        GLuint amount
    ) 
        : Drawable(std::move(vao)), 
          _vbo(std::move(vbo)),
          _vao_triangles(std::move(vao_triangles)),
          _vbo_triangles(std::move(vbo_triangles)),
          _shader( // Big yikes
              Shader::create(
                  ShaderInfo { "shaders/mvm.vert", ShaderType::VERTEX }, 
                  ShaderInfo { "shaders/mvm.frag", ShaderType::FRAGMENT })),
          _amount(amount),
          _marching_cubes(MarchingCubesCompute::create(amount))
    {
    }

    static std::shared_ptr<Particles> create_impl(GLuint amount) {
        auto vao = VertexArrayObject();
        auto vbo = VertexBufferObject(VertexBufferType::ARRAY);

        vao.bind();
        vbo.bind();

        // Reserve data in buffer for number of points
        vbo.send_data_raw(nullptr, amount * sizeof(glm::vec4), StorageType::DYNAMIC);

        vbo.enable_attribute_pointer(0, 4, VertexDataType::FLOAT, 4, 0);
        
        vao.unbind();

        auto vao_triangles = VertexArrayObject();
        auto vbo_triangles = VertexBufferObject(VertexBufferType::ARRAY);

        // vao_triangles.bind();
        // vbo_triangles.bind();

        // vbo_triangles.send_data_raw(nullptr, 100 * sizeof(glm::vec3), StorageType::DYNAMIC);

        // vbo_triangles.enable_attribute_pointer(0, 3, VertexDataType::FLOAT, 3, 0);
        
        // vao_triangles.unbind();

        return std::make_shared<Particles>(std::move(vao), std::move(vbo), std::move(vao_triangles), std::move(vbo_triangles), amount);
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
        _marching_cubes->dispatch(sea_level);    

        glm::vec4 *data = _marching_cubes->expose_points_buffer();
        if(data == nullptr) {
            return;
        }

        // Send updated data to buffer
        _vao.bind();
        _vbo.send_data_raw(data, _amount * sizeof(glm::vec4), StorageType::DYNAMIC);
        _vao.unbind();

        _marching_cubes->close_points_buffer();

        // Triangle *triangles = _marching_cubes->expose_triangle_buffer();
        // if(data == nullptr) {
        //     return;
        // }

        // auto num_triangles = _marching_cubes->num_triangles();

        // std::cout << "Copying " << num_triangles << " triangles!" << std::endl;

        // // Send updated data to buffer
        // _vao_triangles.bind();
        // _vbo_triangles.send_data_raw(data, num_triangles * sizeof(Triangle), StorageType::DYNAMIC);
        // _vao_triangles.unbind();

        // _marching_cubes->close_triangle_buffer();
    }

private:
    VertexBufferObject _vbo;
    VertexArrayObject _vao_triangles;
    VertexBufferObject _vbo_triangles;
    Shader _shader;
    GLsizei _amount;
    std::shared_ptr<MarchingCubesCompute> _marching_cubes;
};