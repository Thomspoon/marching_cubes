#pragma once

#include <cstring>
#include <vector>

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

    explicit AtomicBufferObject(AtomicBufferObject&& other) noexcept
        : _asb(other._asb), _index(other._index)
    {
        other._asb = 0;
        other._index = 0;
    }

    ~AtomicBufferObject() {
        glDeleteBuffers(1, &_asb);
    }

    void clear()
    {
        uint32_t clear = 0;
        GL_CHECK(glInvalidateBufferData(GL_ATOMIC_COUNTER_BUFFER));
        GL_CHECK(glClearBufferData(GL_ATOMIC_COUNTER_BUFFER, GL_R32UI, GL_RED, GL_UNSIGNED_INT, &clear));
    }

    InternalStorageType read()
    {
        uint32_t value = 0;
        GL_CHECK(glGetBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(uint32_t), &value));
        return value;
    }

    // Reserve data inside buffer storage
    void reserve_storage(GLuint size, StorageType type)
    {
        GL_CHECK(glBindBufferBase(InternalBufferType, _index, _asb));
        GL_CHECK(glBufferData(InternalBufferType, size, nullptr, static_cast<GLenum>(type)));
        _size = size;
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

    void bind() const
    {
        GL_CHECK(glBindBuffer(InternalBufferType, _ssb));
    }

    void unbind() const
    {
        GL_CHECK(glBindBuffer(InternalBufferType, 0))
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

    explicit VertexArrayObject(VertexArrayObject&& other) noexcept
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
        glDeleteBuffers(1, &vbo);
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

    void buffer_sub_data(const void* data, uint32_t offset, uint32_t size) const {
        GL_CHECK(glBufferSubData(static_cast<GLenum>(type), offset, size, data));
    }

    template<typename ShaderType>
    void copy_from_ssbo(const ShaderStorageBuffer<ShaderType>& ssbo, uint32_t size) {
        bind();
        ssbo.bind();
        glCopyBufferSubData(GL_SHADER_STORAGE_BUFFER, GL_ARRAY_BUFFER, 0, 0, size);
        unbind();
        ssbo.unbind();
    }

    void unbind() const {
        GL_CHECK(glBindBuffer(static_cast<GLenum>(type), 0));
    }

    VboInner vbo;
    const VertexBufferType type;
};

struct Texture2D {
    Texture2D() : _texture(0u)
    {
        GL_CHECK(glGenTextures(1, &_texture));
    }

    const GLuint get_id() const
    {
        return _texture;
    }

    // TODO: Define mipmap level
    void specify(
        GLint internal_format,
        GLsizei width,
        GLsizei height,
        GLenum format,
        GLenum type
    )
    {
        GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, type, nullptr));
    }

    void set_parameteri(GLenum parameter, GLint value)
    {
        GL_CHECK(glTexParameteri(GL_TEXTURE_2D, parameter, value));
    }

    void bind()
    {
        GL_CHECK(glBindTexture(GL_TEXTURE_2D, _texture));
    }

    GLuint _texture;
};

struct FramebufferObject {
    static FramebufferObject create() {
        GLuint fbo;
        GL_CHECK(glGenFramebuffers(1, &fbo));
        return FramebufferObject(fbo);
    }

    explicit FramebufferObject(FramebufferObject&& other) noexcept
        : _fbo(other._fbo)
    {
        other._fbo = 0;
    }

    ~FramebufferObject() {
        glDeleteFramebuffers(1, &_fbo);
    }

    void bind() const
    {
        GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, _fbo));
    }

    void unbind() const
    {
        GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    }

    void attach_texture(const Texture2D& texture, GLenum texture_type) const
    {
        GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, texture_type, GL_TEXTURE_2D, texture.get_id(), 0));
    }

    void remove_color_buffer() const
    {
        GL_CHECK(glDrawBuffer(GL_NONE));
        GL_CHECK(glReadBuffer(GL_NONE));
    }

private:
    explicit FramebufferObject(GLuint fbo) : _fbo(fbo) {}

    GLuint _fbo;
};
