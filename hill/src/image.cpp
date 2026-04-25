#include "hill/image.hpp"

#include <stb_image.h>

#include <utility>

namespace hill::image {
    Image::Image(const std::string& file_buffer) {
        m_data = stbi_load_from_memory(
            reinterpret_cast<const unsigned char*>(file_buffer.data()),
            int (file_buffer.size()),
            &m_width,
            &m_height,
            &m_channels,
            0
        );

        if (!m_data) {
            throw ImageError("Could not load image");
        }
    }

    Image::~Image() {
        stbi_image_free(m_data);
    }

    Image::Image(Image&& other) noexcept
        : m_data(std::exchange(other.m_data, nullptr)) {}

    Image& Image::operator=(Image&& other) noexcept {
        m_data = std::exchange(other.m_data, nullptr);
        return *this;
    }
}
