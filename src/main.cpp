#include <iostream>
#include <limits>

#include "camera.hpp"
#include "computable.hpp"
#include "drawable.hpp"
#include "shader.hpp"
#include "window.hpp"

#include "custom/computables/marching_cubes.hpp"
#include "custom/drawables/cube.hpp"
#include "custom/drawables/terrain_chunk.hpp"

#include "glm/glm.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// settings
constexpr auto WINDOW_WIDTH = 1440;
constexpr auto WINDOW_HEIGHT = 900;

auto camera_settings = CameraSettings(CameraDefault::ZOOM, WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 1000.0f);
auto camera = Camera<Perspective>(camera_settings, glm::vec3(0.0f, 10.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), 45.0, 0.0f);

Window window(WINDOW_WIDTH, WINDOW_HEIGHT, "Marching Cubes");

bool focus = true;
bool draw_points = true;
bool debug_view = false;

// custom callback 
void process_input(float delta_time)
{
    using namespace Keyboard;

    if(window.get_key(Key::KEY_ESCAPE) == KeyState::PRESSED) {
        focus = false;
        window.set_mouse_mode(MouseMode::NORMAL);
    }

    if(window.get_key(Key::KEY_W) == KeyState::PRESSED) {
        camera.process_keyboard(CameraMovement::FORWARD, delta_time);
    }

    if(window.get_key(Key::KEY_A) == KeyState::PRESSED) {
        camera.process_keyboard(CameraMovement::LEFT, delta_time);
    }

    if(window.get_key(Key::KEY_S) == KeyState::PRESSED) {
        camera.process_keyboard(CameraMovement::BACKWARD, delta_time);
    }

    if(window.get_key(Key::KEY_D) == KeyState::PRESSED) {
        camera.process_keyboard(CameraMovement::RIGHT, delta_time);
    }

    if(window.get_key(Key::KEY_Q) == KeyState::PRESSED) {
        window.polygon_mode(PolygonMode::FILL);
    }

    if(window.get_key(Key::KEY_E) == KeyState::PRESSED) {
        window.polygon_mode(PolygonMode::LINE);
    }

    if(window.get_key(Key::KEY_P) == KeyState::PRESSED) {
        draw_points = true;
    }

    if (window.get_key(Key::KEY_O) == KeyState::PRESSED) {
        draw_points = false;
    }

    if (window.get_key(Key::KEY_O) == KeyState::PRESSED) {
        draw_points = false;
    }

    if (window.get_key(Key::KEY_O) == KeyState::PRESSED) {
        draw_points = false;
    }

    if (window.get_key(Key::KEY_O) == KeyState::PRESSED) {
        draw_points = false;
    }

    // if (window.get_key(Key::KEY_V) == KeyState::PRESSED) {
    //     debug_view = true;
    // }

    // if (window.get_key(Key::KEY_B) == KeyState::PRESSED) {
    //     debug_view = false;
    // }
}

void process_mouse_button(GLFWwindow* glfw_window, int button, int action, int mods) {
    if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && focus == false && !ImGui::GetIO().WantCaptureMouse) {
        focus = true;
        window.set_mouse_mode(MouseMode::DISABLED);
    }
}

void process_mouse_movement(GLFWwindow* glfw_window, double xpos, double ypos) {
    static auto first_mouse = true;
    static auto last_x = 0.0;
    static auto last_y = 0.0;

    if (first_mouse)
    {
        last_x = xpos;
        last_y = ypos;
        first_mouse = false;
    }

    double xoffset = xpos - last_x;
    double yoffset = ypos - last_y;

    last_x = xpos;
    last_y = ypos;

    if(focus) {
        // TODO: Fix this
        camera.process_mouse_movement(float(xoffset), float(yoffset));
    }
}

// TODO: Remove, just for looking at depth data
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

glm::vec3 eye(7.586, 0.0f, 8.0f);

int main() try {
    window.set_mouse_callback(process_mouse_button, process_mouse_movement);
    window.set_mouse_mode(MouseMode::DISABLED);
    window.enable_capability(Capability::DEPTH_TEST);
    window.enable_capability(Capability::PROGRAM_POINT_SIZE);
    //window.enable_capability(Capability::CULL_FACE);

    GenerationSettings settings, last_settings;
    settings.scale = 0.151;

    // Only use valid cubed integers
    auto const number_of_components = 1000000;
    auto const axis_length = std::cbrt(number_of_components);

    auto const num_chunks_per_axis = 1;

    glm::vec3 light_position = glm::vec3(
        axis_length / 2 * num_chunks_per_axis,
        200.0f,
        axis_length / 2 * num_chunks_per_axis
    );

    auto cube_light = Cube::create(light_position);

    auto marching_cubes_shader = MarchingCubesCompute::create(number_of_components);

    // auto shader_debug(
    //     Shader::create(
    //         ShaderInfo { "shaders/debug_depth.vert", ShaderType::VERTEX }, 
    //         ShaderInfo { "shaders/debug_depth.frag", ShaderType::FRAGMENT })
    // );

    auto chunks = std::vector<TerrainChunk>();
    chunks.reserve(num_chunks_per_axis * num_chunks_per_axis * num_chunks_per_axis);
    for (auto i = 0; i < num_chunks_per_axis; i++)
    {
        for (auto j = 0; j < num_chunks_per_axis; j++)
        {
            for (auto k = 0; k < num_chunks_per_axis; k++)
            {
                chunks.push_back(TerrainChunk::create(
                    marching_cubes_shader,
                    number_of_components,
                    glm::ivec3(i* axis_length - i, j * axis_length - j, k* axis_length - k),
                    window
                ));
            }
        }
    }

    auto delta_time = 0.0f;
    auto last_frame = 0.0f;

    // Setup ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void) io;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window.get_window(), true);
    ImGui_ImplOpenGL3_Init("#version 450");

    bool first = true;

    while (!window.should_close())
    {
        auto current_frame = window.get_elapsed_time();
        delta_time = current_frame - last_frame;
        last_frame = current_frame;

        process_input(delta_time);

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        window.clear_screen();

        auto view = camera.get_view_matrix();
        auto projection = camera.get_projection();

        cube_light.update(light_position);
        cube_light.draw(view, projection);

        if(first || !(last_settings == settings))
        {
            if(first)
            {
                first = false;
            }

            for (auto& chunk : chunks)
            {
                chunk.update(settings);
            }
            last_settings = settings;
        }

        for (auto& chunk : chunks)
        {
            chunk.draw(view, projection, settings, camera, light_position, draw_points, debug_view);
        }

        // if(debug_view)
        // {
        //     //float near_plane = 0.1f, far_plane = 100.0f;
        //     shader_debug.use();
        //     glActiveTexture(GL_TEXTURE0);
        //     glBindTexture(GL_TEXTURE_2D, depth_texture.get_id());

        //     // shader_debug.set_float("near_plane", near_plane);
        //     // shader_debug.set_float("far_plane", far_plane);
        //     renderQuad();
        // }

        ImGui::Begin("Procedural Generation Renderer");

        ImGui::Text("Edit Configuration Parameters");

        ImGui::SliderFloat("Scale:       ", &settings.scale, 0.0f, 5.0f);
        ImGui::SliderFloat("Persistence: ", &settings.persistence, 0.0f, 1.0f);
        ImGui::SliderFloat("Lacunarity:  ", &settings.lacunarity, 0.0f, 5.0f);
        ImGui::SliderInt("Octaves:       ", &settings.octaves, 0, 10);
        ImGui::SliderFloat("Iso Level:   ", &settings.iso_level, 0.0f, 2.0f);
        ImGui::SliderFloat("Light Height", &light_position.y, 0.0f, 500.0f);
        ImGui::SliderFloat("Eye X", &eye.x, 0.0f, 16.0f);
        ImGui::SliderFloat("Eye Y", &eye.y, 0.0f, 16.0f);
        ImGui::SliderFloat("Eye Z", &eye.z, 0.0f, 16.0f);
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();

        ImGui::Render();

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        // swap buffers and poll events
        window.swap_and_poll();
    }

    return 0;
} catch (const std::exception& e) {
    std::cout << e.what() << std::endl;
}
