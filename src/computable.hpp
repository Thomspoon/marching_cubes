#pragma once

#include <cstdint>
#include <iostream>
#include <functional>
#include <memory>
#include <variant>

#include <glad/glad.h>
#include "drawable.hpp"

enum BufferIntent {
    READ = GL_READ_ONLY,
    WRITE = GL_WRITE_ONLY,
};

template<typename Internal>
struct ShaderStorageBuffer {
    using SsbInner = GLuint;

    explicit ShaderStorageBuffer() : _ssb(0u) {
        GL_CHECK(glGenBuffers(1, &_ssb));
    }

    explicit ShaderStorageBuffer(ShaderStorageBuffer&& other) 
        : _ssb(other._ssb) 
    {
        other._ssb = 0;
    }

    ~ShaderStorageBuffer() {
        glDeleteBuffers(1, &_ssb);
    }

    // Reserve data inside buffer storage
    void reserve_storage(size_t size, StorageType type)
    {
        GL_CHECK(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _ssb));
        GL_CHECK(glBufferData(GL_SHADER_STORAGE_BUFFER, size, nullptr, static_cast<GLenum>(type)));
        _size = size;
    }

    Internal *map_buffer(BufferIntent intent) const
    {
        GL_CHECK(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _ssb));
        Internal *buffer = reinterpret_cast<Internal *>(glMapBuffer(
            GL_SHADER_STORAGE_BUFFER,
            static_cast<GLbitfield>(intent)
        ));
        return buffer;
    }

    void unmap_buffer() const
    {
        GL_CHECK(glUnmapBuffer(GL_SHADER_STORAGE_BUFFER));
    }

    // TODO: Create MapBufferRange which lets you specify exact bytes to map

    SsbInner _ssb;
    size_t _size;
};

template <typename Child, typename Internal>
class Computable {
public:

    template <typename... Args>
    static std::shared_ptr<Child> create(Args&&... args) {
        return Child::create_impl(std::forward<Args>(args)...);
    }

    // template <typename... Args>
    // void update(Args&&... args) {
    //     static_cast<Child*>(this)->update_impl(std::forward<Args>(args)...);
    // }

    template <typename... Args>
    void dispatch(Args&&... args) {
        static_cast<Child*>(this)->dispatch_impl(std::forward<Args>(args)...);
    }

protected:
    explicit Computable(ShaderStorageBuffer<Internal>&& ssbo) : _ssbo(std::move(ssbo)) {}
    virtual ~Computable() {}

    const ShaderStorageBuffer<Internal> _ssbo;
};