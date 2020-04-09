#include <iostream>
#include <limits>

#include "camera.hpp"
#include "computable.hpp"
#include "drawable.hpp"
#include "shader.hpp"
#include "window.hpp"

#include "custom/drawables/terrain_chunk.hpp"

#include "glm/glm.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// settings
constexpr auto WINDOW_WIDTH = 1440;
constexpr auto WINDOW_HEIGHT = 900;

auto camera_settings = CameraSettings(CameraDefault::ZOOM, WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 1000.0f);
auto camera = Camera<Perspective>(camera_settings, glm::vec3(-3.0f, 3.0f, -10.0f), glm::vec3(0.0f, 1.0f, 0.0f), 70.0, -10.0f);

Window window(WINDOW_WIDTH, WINDOW_HEIGHT, "Advanced Shaders");

bool focus = true;
bool draw_points = true;

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
        draw_points = !draw_points;
    }
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

int main() try {
    window.set_mouse_callback(process_mouse_button, process_mouse_movement);
    window.set_mouse_mode(MouseMode::DISABLED);
    window.enable_capability(Capability::DEPTH_TEST);
    window.enable_capability(Capability::PROGRAM_POINT_SIZE);

    // Only use valid cubed integers
    auto const number_of_components = 4096;
    auto const axis_length = std::cbrt(number_of_components);

    auto const num_chunks_per_axis = 1;

    std::vector<std::shared_ptr<TerrainChunk>> chunks;//(num_chunks_per_axis * num_chunks_per_axis * num_chunks_per_axis);
    for(auto i = 0; i < num_chunks_per_axis; i++) {
        // for(auto j = 0; j < num_chunks_per_axis; j++) {
        //     for(auto k = 0; k < num_chunks_per_axis; k++) {
                //chunks.push_back(std::move(TerrainChunk::create(number_of_components, glm::ivec3(i * axis_length, j * axis_length, k * axis_length))));
                chunks.push_back(std::move(TerrainChunk::create(number_of_components, glm::ivec3(0, 0, 0))));
        //     }
        // }
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

    GenerationSettings settings, last_settings;

    settings.iso_level = 0.0f;

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

        for(auto& chunk : chunks) {
            chunk->update(settings);
            chunk->draw(view, projection, settings, draw_points);
        }

        ImGui::Begin("Procedural Generation Renderer");

        ImGui::Text("Edit Configuration Parameters");

        ImGui::SliderFloat("Scale:       ", &settings.scale, 0.0f, 5.0f);
        ImGui::SliderFloat("Persistence: ", &settings.persistence, 0.0f, 1.0f);
        ImGui::SliderFloat("Lacunarity:  ", &settings.lacunarity, 0.0f, 5.0f);
        ImGui::SliderInt("Octaves:     ", &settings.octaves, 0, 10);
        ImGui::SliderFloat("Iso Level:   ", &settings.iso_level, 0.0f, 2.0f);
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
