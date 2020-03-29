#pragma once

#include <cstdint>
#include <iostream>
#include <functional>
#include <memory>
#include <variant>

#include <glad/glad.h>
#include "drawable.hpp"

enum class BufferIntent {
    READ = GL_READ_ONLY,
    WRITE = GL_WRITE_ONLY,
};

enum class BufferIntentRange {
    READ = GL_MAP_READ_BIT,
};

// Note: Only supports one atomic integer at the moment
struct AtomicBufferObject {
    using InternalStorageType = GLuint;
    constexpr static auto InternalBufferType = GL_ATOMIC_COUNTER_BUFFER;

    explicit AtomicBufferObject(GLuint index) : _asb(0u), _index(index) {
        GL_CHECK(glGenBuffers(1, &_asb));
    }

    explicit AtomicBufferObject(AtomicBufferObject&& other) 
        : _asb(other._asb), _index(other._index)
    {
        other._asb = 0;
        other._index = 0;
    }

    ~AtomicBufferObject() {
        glDeleteBuffers(1, &_asb);
    }

    // Reserve data inside buffer storage
    void reserve_storage(GLuint size, StorageType type)
    {
        GL_CHECK(glBindBufferBase(InternalBufferType, _index, _asb));
        GL_CHECK(glBufferData(InternalBufferType, size, nullptr, static_cast<GLenum>(type)));
        _size = size;
    }

    GLuint* map_buffer(BufferIntent intent) const
    {
        GL_CHECK(glBindBufferBase(InternalBufferType, _index, _asb));
        auto *buffer = GL_CHECK(glMapBuffer(
            InternalBufferType,
            static_cast<GLbitfield>(intent)
        ));
        return reinterpret_cast<GLuint *>(buffer);
    }

    void unmap_buffer() const
    {
        GL_CHECK(glUnmapBuffer(InternalBufferType));
    }

    // TODO: Create MapBufferRange which lets you specify exact bytes to map

    InternalStorageType _asb;
    GLuint _size;
    GLuint _index;
};

template<typename Internal>
struct ShaderStorageBuffer {
    using InternalStorageType = GLuint;
    constexpr static auto InternalBufferType = GL_SHADER_STORAGE_BUFFER;

    explicit ShaderStorageBuffer(GLuint index) : _ssb(0u), _index(index) {
        GL_CHECK(glGenBuffers(1, &_ssb));
    }

    explicit ShaderStorageBuffer(ShaderStorageBuffer&& other) 
        : _ssb(other._ssb), _index(other._index)
    {
        other._ssb = 0;
        other._index = 0;
    }

    ~ShaderStorageBuffer() {
        glDeleteBuffers(1, &_ssb);
    }

    // Reserve data inside buffer storage
    void reserve_storage(GLuint size, StorageType type)
    {
        GL_CHECK(glBindBufferBase(InternalBufferType, _index, _ssb));
        GL_CHECK(glBufferData(InternalBufferType, size, nullptr, static_cast<GLenum>(type)));
        _size = size;
    }

    Internal *map_buffer(BufferIntent intent) const
    {
        GL_CHECK(glBindBufferBase(InternalBufferType, _index, _ssb));
        auto *buffer = GL_CHECK(glMapBuffer(
            InternalBufferType,
            static_cast<GLbitfield>(intent)
        ));
        return reinterpret_cast<Internal *>(buffer);
    }

    Internal *map_buffer_range(GLintptr offset, GLsizeiptr length, BufferIntentRange intent) const
    {
        if (length == 0) {
            return nullptr;
        }

        GL_CHECK(glBindBufferBase(InternalBufferType, _index, _ssb));
        auto *buffer = GL_CHECK(glMapBufferRange(
            InternalBufferType,
            offset,
            length,
            static_cast<GLbitfield>(intent)
        ));
        return reinterpret_cast<Internal *>(buffer);
    }

    void unmap_buffer() const
    {
        GL_CHECK(glUnmapBuffer(InternalBufferType));
    }

    InternalStorageType _ssb;
    GLuint _size;
    GLuint _index;
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