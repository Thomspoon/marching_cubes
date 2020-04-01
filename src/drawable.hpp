#pragma once

#include <cstdint>
#include <cstring>
#include <iostream>
#include <functional>
#include <memory>
#include <string>
#include <variant>
#include <vector>
#include <type_traits>

#include <glad/glad.h>

#include "wrappers.hpp"

using Indices = std::vector<unsigned int>;

enum class VertexPrimitive : GLenum {
    TRIANGLES = GL_TRIANGLES,
    TRIANGLE_STRIP = GL_TRIANGLE_STRIP,
    POINTS = GL_POINTS,
};

struct DrawArrays {
    VertexPrimitive primitive;
    GLuint first;
    GLsizei count;
};

struct DrawElements {
    VertexPrimitive primitive;
    GLsizei count;
    VertexDataType type;
    Indices& indices;
};

// Custom allows you to call a lambda with custom draw functionality
struct Custom {
    const std::function<void()> func;
};

using DrawType = std::variant<DrawArrays, DrawElements, Custom>;

template <typename Child>
class Drawable {
public:

    template <typename... Args>
    static std::shared_ptr<Child> create(Args&&... args) {
        return Child::create_impl(std::forward<Args>(args)...);
    }

    template <typename... Args>
    void update(Args&&... args) {
        return static_cast<Child*>(this)->update_impl(std::forward<Args>(args)...);
    }

    template <typename... Args>
    void draw(Args&&... args) {
        auto draw_type = static_cast<Child*>(this)->draw_impl(std::forward<Args>(args)...);
        if(std::holds_alternative<DrawArrays>(draw_type)) {
            auto draw_arrays = std::get_if<DrawArrays>(&draw_type);
            GL_CHECK(
                glDrawArrays(
                    static_cast<GLenum>(draw_arrays->primitive), 
                    draw_arrays->first, 
                    draw_arrays->count
                )
            );
        } else if(std::holds_alternative<DrawElements>(draw_type)) {
            auto draw_elements = std::get_if<DrawElements>(&draw_type);
            GL_CHECK(
                glDrawElements(
                    static_cast<GLenum>(draw_elements->primitive), 
                    draw_elements->count, 
                    static_cast<GLenum>(draw_elements->type), 
                    nullptr//&draw_elements->indices[0]
                )
            );
        } else if(std::holds_alternative<Custom>(draw_type)) {
            auto custom = std::get_if<Custom>(&draw_type);
            custom->func();
        }
    }

protected:
    explicit Drawable(VertexArrayObject&& vao) : _vao(std::move(vao)) {}
    virtual ~Drawable() {}

    const VertexArrayObject _vao;
};