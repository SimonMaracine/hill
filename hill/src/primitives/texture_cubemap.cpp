#include "hill/primitives/texture_cubemap.hpp"

#include <glad/gl.h>

#include "hill/primitives_registry.hpp"

namespace hill::texture_cubemap {
    TextureCubemap::TextureCubemap(int width, int height)
        : m_width(width), m_height(height) {
        glGenTextures(1, &m_texture);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_texture);

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        glTexStorage2D(GL_TEXTURE_CUBE_MAP, 1, GL_RGBA8, m_width, m_height);

        primitives_registry::add_primitive(primitives_registry::Primitive::TextureCubemap, m_texture);

        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    }

    TextureCubemap::~TextureCubemap() {
        primitives_registry::remove_primitive(primitives_registry::Primitive::TextureCubemap, m_texture);
        glDeleteTextures(1, &m_texture);
    }

    void TextureCubemap::bind(unsigned int unit) const {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_texture);
    }

    void TextureCubemap::unbind(unsigned int unit) const {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    }

    void TextureCubemap::upload_data(const Data& data) const {
        for (unsigned int i {}; i < 6; i++) {
            glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, 0, 0, m_width, m_height, GL_RGBA, GL_UNSIGNED_BYTE, data.array[i]);
        }
    }
}
