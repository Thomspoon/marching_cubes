#include "texture.hpp"
#include "drawable.hpp"

#include <cassert>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Texture2D::Texture2D(GLuint t_id)
        : id(t_id)
{
}

Texture2D Texture2D::create(const char* texture_name)
 {
    auto width = 0;
    auto height = 0;
    auto nrChannels = 0;
    unsigned char *data = stbi_load(texture_name, &width, &height, &nrChannels, 0);
    assert(data != nullptr);

    return create(width, height, data);
}

Texture2D Texture2D::create(GLuint width, GLuint height, unsigned char *data)
{
    GLuint id;

    // Create Texture
    GL_CHECK(glGenTextures(1, &id));
    
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, id));
    GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data));
    // Set Texture wrap and filter modes
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    // Unbind texture
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));

    return Texture2D(id);
}

void Texture2D::bind() const
{
    glBindTexture(GL_TEXTURE_2D, this->id);
}