#pragma once

#include <cstring>

#include <glad/glad.h>

#include "utility.hpp"

enum class VertexDataType {
    FLOAT = GL_FLOAT,
    UNSIGNED_INT = GL_UNSIGNED_INT
};

enum class BufferIntent {
    READ = GL_READ_ONLY,
    WRITE = GL_WRITE_ONLY,
};

enum class BufferIntentRange {
    READ = GL_MAP_READ_BIT,
    WRITE = GL_MAP_WRITE_BIT,
};

enum class VertexBufferType {
    ARRAY = GL_ARRAY_BUFFER,
    ELEMENT = GL_ELEMENT_ARRAY_BUFFER,
};

enum class StorageType {
    STATIC = GL_STATIC_DRAW,
    DYNAMIC = GL_DYNAMIC_DRAW
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

    void map_buffer_range(const void* data, uint32_t offset, uint32_t size) const {
        void *ptr = GL_CHECK(glMapBufferRange(static_cast<GLenum>(type), offset, size, static_cast<GLbitfield>(BufferIntentRange::WRITE)));
        memcpy(ptr, data, size);
        GL_CHECK(glUnmapBuffer(static_cast<GLenum>(type)));
    }

    template<typename ShaderType>
    void stream_from_ssbo(const ShaderStorageBuffer<ShaderType>& ssbo, uint32_t size) {
        const auto CHUNK_MAX_SIZE = 1024u;

        auto size_sent = 0u;

        std::cout << "Sending " << size << " bytes in " << std::ceil(size / CHUNK_MAX_SIZE) << " chunks" << std::endl; 

        auto chunk_number = 1u;
        while(size_sent < size) {
            auto chunk_size = ((size - size_sent) > CHUNK_MAX_SIZE) ? CHUNK_MAX_SIZE : size - size_sent;

            std::cout << "   Chunk #" << chunk_number << " sending " << chunk_size << " bytes!" << std::endl;

            auto chunk = ssbo.map_buffer_range(size_sent, chunk_size, BufferIntentRange::READ);
            map_buffer_range(chunk, size_sent, chunk_size);
            ssbo.unmap_buffer();
            size_sent += chunk_size;
        }
    }

    void unbind() const {
        GL_CHECK(glBindBuffer(static_cast<GLenum>(type), 0));
    }

    VboInner vbo;
    const VertexBufferType type;
};