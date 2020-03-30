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
          _shader_points( // Big yikes
              Shader::create(
                  ShaderInfo { "shaders/grid_points.vert", ShaderType::VERTEX }, 
                  ShaderInfo { "shaders/grid_points.frag", ShaderType::FRAGMENT })),
          _shader_triangles( // Big yikes
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

        vao_triangles.bind();
        vbo_triangles.bind();

        vbo_triangles.send_data_raw(nullptr, 4 * sizeof(Triangle), StorageType::DYNAMIC);

        vbo_triangles.enable_attribute_pointer(0, 3, VertexDataType::FLOAT, 3, 0);

        vao_triangles.unbind();

        return std::make_shared<Particles>(std::move(vao), std::move(vbo), std::move(vao_triangles), std::move(vbo_triangles), amount);
    }

    DrawType draw_impl(glm::mat4& view, glm::mat4& projection, GenerationSettings& settings) const {
        auto draw_type = Custom {
            [this, view, projection, settings](){
                _vao.bind();
                // Draw points
                _shader_points.use();
                _shader_points.set_float("iso_level", settings.iso_level);
                _shader_points.set_mat4("projection", projection);
                _shader_points.set_mat4("view", view);
                _shader_points.set_mat4("model", glm::translate(glm::mat4x4(1.0), glm::vec3(0.0f, 0.0f, 0.0f)));
                glDrawArrays(GL_POINTS, 0, _amount);
                _vao.unbind();

                _vao_triangles.bind();
                // Draw Triangles
                _shader_triangles.use();
                _shader_triangles.set_mat4("projection", projection);
                _shader_triangles.set_mat4("view", view);
                _shader_triangles.set_mat4("model", glm::translate(glm::mat4x4(1.0), glm::vec3(0.0f, 0.0f, 0.0f)));
                glDrawArrays(GL_TRIANGLES, 0, _amount_triangles * 9);
                _vao_triangles.unbind();

                std::cout << "Drawing " << _amount << " points and " << _amount_triangles << " triangles!" << std::endl;
            }
        };

        return DrawType(draw_type);
    }

    void update(GenerationSettings& settings)
    {
        _marching_cubes->dispatch(settings);    

        glm::vec4 *data = _marching_cubes->expose_points_buffer();
        if(data == nullptr) {
            return;
        }

        // Print point noise values
        glm::vec4 (*point)[2][2][2] = reinterpret_cast<glm::vec4(*)[2][2][2]>(data);
        for(auto i = 0u; i < 2; i++) {
            for(auto j = 0u; j < 2; j++) {
                for(auto k = 0u; k < 2; k++) {
                    
                    std::cout << "Point  " << "(" << (*point)[i][j][k].x << ", " << (*point)[i][j][k].y << ", " << (*point)[i][j][k].z << "): ";
                    std::cout << (*point)[i][j][k].w << std::endl;
                }
            }
        }

        // Calculate cube_index
        auto cube_index = 0u;
        if((*point)[0][0][0].w < settings.iso_level) {
            cube_index |= 1;
        }
        if((*point)[1][0][0].w < settings.iso_level) {
            cube_index |= (1 << 1);
        }
        if((*point)[1][0][1].w < settings.iso_level) {
            cube_index |= (1 << 2);
        }
        if((*point)[0][0][1].w < settings.iso_level) {
            cube_index |= (1 << 3);
        }
        if((*point)[0][1][0].w < settings.iso_level) {
            cube_index |= (1 << 4);
        }
        if((*point)[1][1][0].w < settings.iso_level) {
            cube_index |= (1 << 5);
        }
        if((*point)[1][1][1].w < settings.iso_level) {
            cube_index |= (1 << 6);
        }
        if((*point)[0][1][1].w < settings.iso_level) {
            cube_index |= (1 << 7);
        }

        std::cout << "cube_index: " << cube_index << std::endl;

        // Send updated data to buffer
        _vao.bind();
        _vbo.bind();
        _vbo.send_data_raw(data, _amount * sizeof(glm::vec4), StorageType::DYNAMIC);
        _vao.unbind();

        _marching_cubes->close_points_buffer();

        float *triangles = reinterpret_cast<float *>(_marching_cubes->expose_triangle_buffer());
        if(triangles == nullptr) {
            return;
        }

        auto num_triangles = _marching_cubes->num_triangles();
        _amount_triangles = num_triangles;

        // for(auto i = 0u; i < num_triangles; i++) {
        //     std::cout << "Triangle " << i << ": " << std::endl;
        //     std::cout << "(" << triangles[i].vertexA.x << ", " << triangles[i].vertexA.y << ", " << triangles[i].vertexA.z << ") ";
        //     std::cout << "(" << triangles[i].vertexB.x << ", " << triangles[i].vertexB.y << ", " << triangles[i].vertexB.z << ") ";
        //     std::cout << "(" << triangles[i].vertexC.x << ", " << triangles[i].vertexC.y << ", " << triangles[i].vertexC.z << ")";
        //     std::cout << std::endl;
        // }

        // Triangle *triangles = expose_triangle_buffer();
        // if(triangles == nullptr) {
        //     return;
        // }

        std::cout << "Triangle output: ";
        for(auto i = 0u; i < num_triangles * 18; i++)
        {
            std::cout << triangles[i] << ", ";
        }
        std::cout << std::endl;

        std::cout << "Copying " << num_triangles << " triangles!" << std::endl;

        if(num_triangles == 0) {
            return;
        }

        // Send updated data to buffer
        _vao_triangles.bind();
        _vbo_triangles.bind();
        _vbo_triangles.update_data_raw(triangles, 36);
        _vao_triangles.unbind();

        _marching_cubes->close_triangle_buffer();
    }

private:
    VertexBufferObject _vbo;
    VertexArrayObject _vao_triangles;
    VertexBufferObject _vbo_triangles;
    Shader _shader_points;
    Shader _shader_triangles;
    GLsizei _amount;
    GLsizei _amount_triangles;
    std::shared_ptr<MarchingCubesCompute> _marching_cubes;
};