#include "hill/primitives/texture2d.hpp"

#include <utility>

#include <glad/gl.h>
#include <glm/gtc/type_ptr.hpp>

#include "hill/primitives_registry.hpp"

namespace hill::texture2d {
    static constexpr int MIPMAPPING_LEVELS = 4;

    static constexpr int wrap_enum(Wrap wrap) {
        switch (wrap) {
            case Wrap::Repeat:
                return GL_REPEAT;
            case Wrap::MirroredRepeat:
                return GL_MIRRORED_REPEAT;
            case Wrap::ClampToEdge:
                return GL_CLAMP_TO_EDGE;
            case Wrap::ClampToBorder:
                return GL_CLAMP_TO_BORDER;
        }

        std::unreachable();
    }

    static constexpr int filter_enum(Filter filter) {
        switch (filter) {
            case Filter::Nearest:
                return GL_NEAREST;
            case Filter::Linear:
                return GL_LINEAR;
        }

        std::unreachable();
    }

    Texture2D::Texture2D(Format format, int width, int height, bool mipmapping)
        : m_format(format), m_width(width), m_height(height), m_mipmapping(mipmapping) {
        glGenTextures(1, &m_texture);
        primitives_registry::Registry::get().add_texture2d(m_texture);

        allocate_storage();
    }

    Texture2D::~Texture2D() {
        primitives_registry::Registry::get().remove_texture2d(m_texture);
        glDeleteTextures(1, &m_texture);
    }

    void Texture2D::bind(unsigned int unit) const {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, m_texture);
    }

    void Texture2D::unbind(unsigned int unit) const {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void Texture2D::configure(const Specification& specification) const {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_enum(specification.wrap_s));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_enum(specification.wrap_t));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter_enum(specification.min_filter));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter_enum(specification.mag_filter));

        if (specification.wrap_s == Wrap::ClampToBorder || specification.wrap_t == Wrap::ClampToBorder) {
            glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(specification.border_color));
        }
    }

    void Texture2D::upload_data(const void* data) const {
        switch (m_format) {
            case Format::Rgba8:
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width, m_height, GL_RGBA, GL_UNSIGNED_BYTE, data);
                break;
        }

        if (m_mipmapping) {
            glGenerateMipmap(GL_TEXTURE_2D);
        }
    }

    void Texture2D::allocate_storage() const {
        glBindTexture(GL_TEXTURE_2D, m_texture);

        switch (m_format) {
            case Format::Rgba8:
                glTexStorage2D(GL_TEXTURE_2D, m_mipmapping ? MIPMAPPING_LEVELS : 1, GL_RGBA8, m_width, m_height);
                break;
        }

        glBindTexture(GL_TEXTURE_2D, 0);
    }
}
