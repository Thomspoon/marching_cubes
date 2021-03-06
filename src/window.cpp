#include <iostream>

#include "window.hpp"

// Default callback will just resize the OpenGL viewport
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    //int width;
    //int height;
    //glfwGetFramebufferSize(window, &width, &height);
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

void GLAPIENTRY
MessageCallback( GLenum source,
                 GLenum type,
                 GLuint id,
                 GLenum severity,
                 GLsizei length,
                 const GLchar* message,
                 const void* userParam )
{
    if(type != GL_DEBUG_TYPE_ERROR) {
        return;
    }

    fprintf( stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
           ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
            type, severity, message );
}

Window::Window(const unsigned int width, const unsigned int height, const char *window_name) {
    // glfw: initialize and configure
    // ------------------------------
    glfwSetErrorCallback(glfw_error_callback);
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Create internal glfw window
    GLFWwindow* window = glfwCreateWindow(width, height, window_name, nullptr, nullptr);
    if (window == nullptr)
    {
        glfwTerminate();
        throw std::runtime_error("Failed to initialize GLFW");
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Load all function pointers
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        glfwTerminate();
        throw std::runtime_error("Failed to initialize GLAD");
    }

    // During init, enable debug output
    glEnable              ( GL_DEBUG_OUTPUT );
    glDebugMessageCallback( MessageCallback, 0 );

    _window = window;
}

Window::~Window() {
    glfwTerminate();
}

void Window::clear_screen() {
    // TODO: Set custom clear color
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    // TODO: Turn on/off clear bits
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

bool Window::should_close() {
    return glfwWindowShouldClose(_window);
}

GLFWwindow* Window::get_window() {
    return _window;
}

void Window::enable_capability(Capability capability) {
    glEnable(static_cast<GLenum>(capability));
}

void Window::polygon_mode(PolygonMode mode) {
    glPolygonMode(GL_FRONT_AND_BACK, static_cast<GLenum>(mode));
}

void Window::close() {
    glfwSetWindowShouldClose(_window, true);
}

void Window::poll_events() {
    glfwPollEvents();
}

void Window::swap_and_poll() {
    glfwSwapBuffers(_window);
    glfwPollEvents();
}

Keyboard::KeyState Window::get_key(Keyboard::Key key) {
    return static_cast<Keyboard::KeyState>(glfwGetKey(_window, static_cast<GLint>(key)));
}

float Window::get_elapsed_time() {
    return static_cast<float>(glfwGetTime());
}

void Window::set_mouse_callback(GLFWmousebuttonfun mouse_btn_func, GLFWcursorposfun mouse_pos_func) {
    if(mouse_btn_func) {
        glfwSetMouseButtonCallback(_window, mouse_btn_func);
    }

    if(mouse_pos_func) {
        glfwSetCursorPosCallback(_window, mouse_pos_func);
    }
}

void Window::set_mouse_mode(MouseMode mode) {
    glfwSetInputMode(_window, GLFW_CURSOR, static_cast<int>(mode));
}

void Window::set_zoom_callback(GLFWscrollfun func) {
    glfwSetScrollCallback(_window, func);
}