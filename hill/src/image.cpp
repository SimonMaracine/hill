#include "hill/image.hpp"

#include <stb_image.h>

#include <utility>

namespace hill::image {
    Image::Image(const file::Buffer& buffer) {
        load(reinterpret_cast<const unsigned char*>(buffer.data()), buffer.size());
    }

    Image::Image(const unsigned char* data, std::size_t size) {
        load(data, size);
    }

    Image::~Image() {
        stbi_image_free(m_data);
    }

    void Image::load(const unsigned char* data, std::size_t size) {
        m_data = stbi_load_from_memory(
            data,
            int(size),
            &m_width,
            &m_height,
            &m_channels,
            STBI_rgb_alpha
        );

        if (!m_data) {
            throw ImageError("Could not load image");
        }
    }

    Image::Image(Image&& other) noexcept
        : m_width(other.m_width), m_height(other.m_height), m_channels(other.m_channels), m_data(std::exchange(other.m_data, nullptr)) {}

    Image& Image::operator=(Image&& other) noexcept {
        m_width = other.m_width;
        m_height = other.m_height;
        m_channels = other.m_channels;
        m_data = std::exchange(other.m_data, nullptr);
        return *this;
    }
}
