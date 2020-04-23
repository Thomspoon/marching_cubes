// Adapted from the following resource:
// https://learnopengl.com/code_viewer_gh.php?code=includes/learnopengl/camera.h

#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <memory>
#include <vector>

struct CameraSettings {
public:
    CameraSettings(float zoom, float aspect_ratio, float near, float far);

    float zoom;
    float aspect_ratio;
    float near;
    float far;
};

class CameraProjection {
public:
    CameraProjection(CameraSettings settings) 
        : _settings(settings)
    {
    }

    virtual ~CameraProjection(){}

    virtual glm::mat4 get_projection() = 0;

    CameraSettings& get_settings() {
        return _settings;
    }

private:
    CameraSettings _settings;
};

class Perspective : private CameraProjection {
public:
    Perspective(CameraSettings settings) 
        : CameraProjection(settings)
    {
    }

    glm::mat4 get_projection() {
        auto settings = get_settings();
        return glm::perspective(
            glm::radians(settings.zoom),
            settings.aspect_ratio,
            settings.near,
            settings.far
        );
    }
};

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum class CameraMovement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

// Default camera values
namespace CameraDefault {
    constexpr float YAW         = -90.0f;
    constexpr float PITCH       =  0.0f;
    constexpr float SPEED       =  50.0f;
    constexpr float SENSITIVITY =  0.1f;
    constexpr float ZOOM        =  45.0f;
}

// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
template<typename Projection>
class Camera
{
public:
    Camera(
        CameraSettings settings,
        glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), 
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), 
        float yaw = CameraDefault::YAW, 
        float pitch = CameraDefault::PITCH
    )
      : _front(glm::vec3(0.0f, 0.0f, -1.0f)), 
        _movement_speed(CameraDefault::SPEED), 
        _mouse_sensitivity(CameraDefault::SENSITIVITY),
        _projection(std::make_unique<Projection>(settings))
    {
        _position = position;
        _world_up = up;
        _yaw = yaw;
        _pitch = pitch;

        update_camera_vectors();
    }

    glm::mat4 get_view_matrix() {
        return glm::lookAt(_position, _position + _front, _up);
    }

    glm::mat4 get_projection() {
        return _projection->get_projection();
    }

    glm::vec3 get_position() {
        return _position;
    }

    void process_keyboard(CameraMovement direction, float delta_time) {
        float velocity = _movement_speed * delta_time;
        if (direction == CameraMovement::FORWARD)
            _position += _front * velocity;
        if (direction == CameraMovement::BACKWARD)
            _position -= _front * velocity;
        if (direction == CameraMovement::LEFT)
            _position -= _right * velocity;
        if (direction == CameraMovement::RIGHT)
            _position += _right * velocity;
    }

    void process_mouse_movement(float xoffset, float yoffset, GLboolean constrain_pitch = true) {
        xoffset *= _mouse_sensitivity;
        yoffset *= _mouse_sensitivity;

        _yaw   += xoffset;
        _pitch += yoffset;

        // Make sure that when pitch is out of bounds, screen doesn't get flipped
        if (constrain_pitch)
        {
            if (_pitch > 89.0f) {
                _pitch = 89.0f;
            }

            if (_pitch < -89.0f) {
                _pitch = -89.0f;
            }
        }

        // Update _front, _right and _up vectors using the updated Euler angles
        update_camera_vectors();
    }

    void process_mouse_scroll(float yoffset) {
        auto settings = _projection->get_settings();
        if (settings.zoom >= 1.0f && settings.zoom <= 45.0f) {
            settings.zoom -= yoffset;
        }

        if (settings.zoom <= 1.0f) {
            settings.zoom = 1.0f;
        }

        if (settings.zoom >= 45.0f) {
            settings.zoom = 45.0f;
        }
    }

private:
    // Camera Attributes
    glm::vec3 _position;
    glm::vec3 _front;
    glm::vec3 _up;
    glm::vec3 _right;
    glm::vec3 _world_up;

    // Euler Angles
    float _yaw;
    float _pitch;
    
    // Camera options
    float _movement_speed;
    float _mouse_sensitivity;

    // Calculates the front vector from the Camera's (updated) Euler Angles
    void update_camera_vectors() {
        // Calculate the new _front vector
        glm::vec3 front;
        front.x = cos(glm::radians(_yaw)) * cos(glm::radians(_pitch));
        front.y = sin(glm::radians(_pitch));
        front.z = sin(glm::radians(_yaw)) * cos(glm::radians(_pitch));
        _front = glm::normalize(front);

        // Also re-calculate the _right and _up vector
        _right = glm::normalize(glm::cross(_front, _world_up));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        _up    = glm::normalize(glm::cross(_right, _front));
    }

    std::unique_ptr<Projection> _projection;
};
