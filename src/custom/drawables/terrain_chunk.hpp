#pragma once

#include "../../drawable.hpp"
#include "../../shader.hpp"
#include "../../texture.hpp"
#include "../computables/marching_cubes.hpp"

// TerrainChunk will receive their vertex attributes after compute shader processes
// positions
class TerrainChunk : public Drawable<TerrainChunk> {
public:
    TerrainChunk(
        VertexArrayObject&& vao_points,
        VertexBufferObject&& vbo_points,
        VertexArrayObject&& vao_triangles, 
        VertexBufferObject&& vbo_triangles, 
        GLuint amount_points,
        glm::ivec3 origin,
        std::shared_ptr<MarchingCubesCompute> compute_shader
    ) : _vao_points(std::move(vao_points)),
        _vbo_points(std::move(vbo_points)),
        _vao_triangles(std::move(vao_triangles)),
        _vbo_triangles(std::move(vbo_triangles)),
        _shader_points(
            Shader::create(
                ShaderInfo { "shaders/grid_points.vert", ShaderType::VERTEX }, 
                ShaderInfo { "shaders/grid_points.frag", ShaderType::FRAGMENT })),
        _shader_triangles(
            Shader::create(
                ShaderInfo { "shaders/light_mvm.vert", ShaderType::VERTEX }, 
                ShaderInfo { "shaders/light_mvm.frag", ShaderType::FRAGMENT })),
        _amount_points(amount_points),
        _origin(origin),
        _marching_cubes(compute_shader)
    {
    }

    static TerrainChunk create
    (
        std::shared_ptr<MarchingCubesCompute> compute_shader, 
        GLuint num_points, 
        glm::ivec3 origin
    )
    {
        auto vao_points = VertexArrayObject();
        auto vbo_points = VertexBufferObject(VertexBufferType::ARRAY);

        vao_points.bind();
        vbo_points.bind();

        // Reserve data in buffer for number of points
        vbo_points.send_data_raw(nullptr, num_points * sizeof(glm::vec4), StorageType::DYNAMIC);

        vbo_points.enable_attribute_pointer(0, 4, VertexDataType::FLOAT, 4, 0);

        vao_points.unbind();

        auto vao_triangles = VertexArrayObject();
        auto vbo_triangles = VertexBufferObject(VertexBufferType::ARRAY);

        vao_triangles.bind();
        vbo_triangles.bind();

        // Every cube has the chance for up to 5 triangles, however rarely is every cube going to have
        // that many. Need some way to extrapolate worst case scenario
        vbo_triangles.send_data_raw(nullptr, num_points * 2 * sizeof(Triangle), StorageType::DYNAMIC);

        vbo_triangles.enable_attribute_pointer(0, 4, VertexDataType::FLOAT, 12, 0);
        vbo_triangles.enable_attribute_pointer(1, 4, VertexDataType::FLOAT, 12, 4);
        vbo_triangles.enable_attribute_pointer(2, 4, VertexDataType::FLOAT, 12, 8);

        vao_triangles.unbind();

        return TerrainChunk(
            std::move(vao_points),
            std::move(vbo_points),
            std::move(vao_triangles),
            std::move(vbo_triangles),
            num_points,
            origin,
            compute_shader
        );
    }

    void draw(
        glm::mat4& view,
        glm::mat4& projection,
        GenerationSettings& settings,
        glm::vec3 camera_position,
        glm::vec3 light_position,
        bool draw_points
    ) const 
    {
        if (draw_points)
        {
            _vao_points.bind();
            // Draw points
            _shader_points.use();
            _shader_points.set_float("iso_level", settings.iso_level);
            _shader_points.set_mat4("projection", projection);
            _shader_points.set_mat4("view", view);
            _shader_points.set_mat4("model", glm::translate(glm::mat4x4(1.0), glm::vec3(0.0f, 0.0f, 0.0f)));
            glDrawArrays(GL_POINTS, 0, _amount_points);
            _vao_points.unbind();
        }

        _vao_triangles.bind();
        // Draw Triangles
        _shader_triangles.use();
        _shader_triangles.set_mat4("projection", projection);
        _shader_triangles.set_mat4("view", view);
        _shader_triangles.set_mat4("model", glm::translate(glm::mat4x4(1.0), glm::vec3(0.0f, 0.0f, 0.0f)));
        _shader_triangles.set_vec3("light_pos", light_position);
        _shader_triangles.set_vec3("view_pos", camera_position);
        _shader_triangles.set_vec3("light_color", glm::vec3(1.0f, 1.0f, 1.0f));
        glDrawArrays(GL_TRIANGLES, 0, _amount_triangles * 3);
        _vao_triangles.unbind();
    }

    void update(GenerationSettings& settings)
    {
        _marching_cubes->dispatch(settings, _origin, int(std::cbrt(_amount_points)));

        _vao_points.bind();
        _vbo_points.bind();
        _vbo_points.stream_from_ssbo(_marching_cubes->points_buffer(), _amount_points * sizeof(glm::vec4));
        _vao_points.unbind();

        auto num_triangles = _marching_cubes->num_triangles();
        _amount_triangles = num_triangles;

        if(num_triangles == 0) {
            return;
        }

        _vao_triangles.bind();
        _vbo_triangles.bind();
        _vbo_triangles.stream_from_ssbo(_marching_cubes->triangle_buffer(), _amount_triangles * sizeof(Triangle));
        _vao_triangles.unbind();
    }

private:
    VertexArrayObject _vao_points;
    VertexBufferObject _vbo_points;
    VertexArrayObject _vao_triangles;
    VertexBufferObject _vbo_triangles;
    Shader _shader_points;
    Shader _shader_triangles;
    GLsizei _amount_points;
    glm::ivec3 _origin;
    GLsizei _amount_triangles;
    std::shared_ptr<MarchingCubesCompute> _marching_cubes;
};