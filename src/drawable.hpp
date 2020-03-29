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

#define __DEBUG__

// This is a convenience define that will check for errors after each hidden
// opengl call, and if API_DUMP is defined, it will also log gl calls 
// to stdout
#define API_DUMP 0
#ifdef __DEBUG__
#define GL_CHECK(func) func;                                                \
if(API_DUMP) {                                                              \
    std::cout << #func << std::endl;                                        \
}
#else
#define GL_CHECK(var) var;
#endif

using Indices = std::vector<unsigned int>;

enum class VertexDataType {
    FLOAT = GL_FLOAT,
    UNSIGNED_INT = GL_UNSIGNED_INT
};

struct VertexArrayObject {
    using VaoInner = GLuint;

    explicit VertexArrayObject() : _vao(0u) {
        GL_CHECK(glGenVertexArrays(1, &_vao));
    }

    explicit VertexArrayObject(VertexArrayObject&& other) 
        : _vao(other._vao) 
    {
        other._vao = 0;
    }

    ~VertexArrayObject() {
        glDeleteVertexArrays(1, &_vao);
    }

    void bind() const {
        GL_CHECK(glBindVertexArray(_vao));
    }

    void unbind() const {
        GL_CHECK(glBindVertexArray(0));
    }

    VaoInner _vao;
};

enum class VertexBufferType {
    ARRAY = GL_ARRAY_BUFFER,
    ELEMENT = GL_ELEMENT_ARRAY_BUFFER,
};

enum class StorageType {
    STATIC = GL_STATIC_DRAW,
    DYNAMIC = GL_DYNAMIC_DRAW
};

struct VertexBufferObject {
    using VboInner = GLuint;

    explicit VertexBufferObject(const VertexBufferType t_type) : vbo(0u), type(t_type) {
        GL_CHECK(glGenBuffers(1, &vbo));
    }

    explicit VertexBufferObject(VertexBufferObject&& other) 
        : vbo(other.vbo), type(other.type)
    {
        other.vbo = 0;
    }

    ~VertexBufferObject() {
        glGenBuffers(1, &vbo);
    }

    void enable_attribute_pointer(GLsizei index, GLint size, VertexDataType t_type, GLuint stride, std::size_t offset) {
        auto width = 0u;
        switch(t_type) {
            case VertexDataType::FLOAT:
            case VertexDataType::UNSIGNED_INT:
            {
                width = 4;
            }
            break;
        }

        GL_CHECK(glVertexAttribPointer(index, size, static_cast<GLenum>(t_type), GL_FALSE, stride * width, reinterpret_cast<void *>(offset * width)));
        GL_CHECK(glEnableVertexAttribArray(index));
    }

    void bind() const {
        GL_CHECK(glBindBuffer(static_cast<GLenum>(type), vbo));
        std::cout << "Binding: " << vbo << std::endl;
    }

    template<typename Type>
    void send_data(const std::vector<Type> &data, const StorageType draw_type) const {
        GL_CHECK(glBufferData(static_cast<GLenum>(type), sizeof(Type) * data.size(), &data[0], static_cast<GLenum>(draw_type)));
    }

    template <typename Type, std::size_t Size>
    void send_data(const Type (&data)[Size], const StorageType draw_type) const {
        GL_CHECK(glBufferData(static_cast<GLenum>(type), Size * sizeof(data[0]), &data[0], static_cast<GLenum>(draw_type)));
    }

    void send_data_raw(const void* data, GLsizei size, const StorageType draw_type) const {
        GL_CHECK(glBufferData(static_cast<GLenum>(type), size, data, static_cast<GLenum>(draw_type)));
    }

    template<typename Type>
    void update_data(const std::vector<Type> &data) const {
        void *ptr = GL_CHECK(glMapBuffer(static_cast<GLenum>(type), GL_WRITE_ONLY));
        memcpy(ptr, &data[0], sizeof(Type) * data.size());
        GL_CHECK(glUnmapBuffer(static_cast<GLenum>(type)));
    }

    void update_data_raw(const void* data, size_t size) const {
        void *ptr = GL_CHECK(glMapBuffer(static_cast<GLenum>(type), GL_WRITE_ONLY));
        memcpy(ptr, data, size);
        GL_CHECK(glUnmapBuffer(static_cast<GLenum>(type)));
    }

    void unbind() const {
        GL_CHECK(glBindBuffer(static_cast<GLenum>(type), 0));
    }

    VboInner vbo;
    const VertexBufferType type;
};

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