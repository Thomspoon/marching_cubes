#pragma once

#include <memory>

#include "glm/glm.hpp"

#include "../../drawable.hpp"
#include "../../shader.hpp"
#include "../../texture.hpp"
#include "../computables/marching_cubes.hpp"
#include "../../window.hpp"

// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

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
        // Texture2D&& depth_texture,
        // FramebufferObject&& depth_fbo,
        // Window& window
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
                ShaderInfo { "shaders/gourad.vert", ShaderType::VERTEX }, 
                ShaderInfo { "shaders/gourad.frag", ShaderType::FRAGMENT })),
        _amount_points(amount_points),
        _origin(origin),
        _marching_cubes(compute_shader)
        // _depth_texture(std::move(depth_texture)),
        // _depth_fbo(std::move(depth_fbo)),
        // _shader_depth(
        //     Shader::create(
        //         ShaderInfo { "shaders/depth_map.vert", ShaderType::VERTEX })
        // ),
        // _temp_debug(
        //     Shader::create(
        //         ShaderInfo { "shaders/debug_quad.vert", ShaderType::VERTEX }, 
        //         ShaderInfo { "shaders/debug_quad.frag", ShaderType::FRAGMENT })),
        // _window(window)
    {
    }

    static TerrainChunk create
    (
        std::shared_ptr<MarchingCubesCompute> compute_shader, 
        GLuint num_points, 
        glm::ivec3 origin
        //Window& window,
        // Texture2D& texture,
        // FramebufferObject& depth_fbo
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
            //std::move(texture),
            //std::move(depth_fbo),
            //window
        );
    }

    void draw(
        glm::mat4& view,
        glm::mat4& projection,
        GenerationSettings& settings,
        glm::vec3 camera_position,
        glm::vec3 light_position,
        bool draw_points
    ) 
    {
        //float near_plane = 1.0f, far_plane = 7.5f;
        ////float near_plane = 0.1f, far_plane = 1000.0f;
        //glm::mat4 light_projection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
        //glm::mat4 light_view = glm::lookAt(light_position, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
        //glm::mat4 light_space_matrix = light_projection * light_view;
        //_shader_depth.use();
        //_shader_depth.set_mat4("light_space_matrix", light_space_matrix);

        //glViewport(0, 0, 1024, 1024);
        //_depth_fbo.bind();
        //glClear(GL_DEPTH_BUFFER_BIT);
        //glActiveTexture(GL_TEXTURE0);
        //glBindTexture(GL_TEXTURE_2D, _depth_texture.get_id());

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
        //_depth_fbo.unbind();

        //// TODO: Set custom clear color
        //glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        //// TODO: Turn on/off clear bits
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //// Reset viewport dimensions
        //int width, height;
        //_window.get_dimensions(width, height);
        //glViewport(0, 0, width, height);

        //_temp_debug.use();
        //_temp_debug.set_float("near_plane", near_plane);
        //_temp_debug.set_float("far_plane", far_plane);
        //glActiveTexture(GL_TEXTURE0);
        //glBindTexture(GL_TEXTURE_2D, _depth_texture.get_id());
        //renderQuad();
    }

    void update(GenerationSettings& settings)
    {
        _marching_cubes->dispatch(settings, _origin, int(std::cbrt(_amount_points)));

        _vao_points.bind();
        _vbo_points.bind();
        _vbo_points.copy_from_ssbo(_marching_cubes->points_buffer(), _amount_points * sizeof(glm::vec4));
        _vao_points.unbind();

        auto num_triangles = _marching_cubes->num_triangles();
        _amount_triangles = num_triangles;

        if(num_triangles == 0) {
            return;
        }

        _vao_triangles.bind();
        _vbo_triangles.bind();
        _vbo_triangles.copy_from_ssbo(_marching_cubes->triangle_buffer(), _amount_triangles * sizeof(Triangle));
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

    // Texture2D& _depth_texture;
    // FramebufferObject& _depth_fbo;
    // Shader _shader_depth;
    // Shader _temp_debug;

    // Window& _window;
};