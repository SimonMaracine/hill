#pragma once

#include "hill/utility.hpp"
#include "hill/error.hpp"

namespace hill::image {
    class Image {
    public:
        Image() = default;
        explicit Image(const utility::Buffer& buffer);
        Image(const unsigned char* data, std::size_t size);
        ~Image();

        Image(const Image&) = delete;
        Image& operator=(const Image&) = delete;
        Image(Image&& other) noexcept;
        Image& operator=(Image&& other) noexcept;

        int width() const { return m_width; }
        int height() const { return m_height; }
        int channels() const { return m_channels; }
        unsigned char* data() const { return m_data; }

        operator bool() const { return m_data; }
    private:
        void load(const unsigned char* data, std::size_t size);

        int m_width {};
        int m_height {};
        int m_channels {};
        unsigned char* m_data {};
    };

    struct ImageError : error::Error {
        using Error::Error;
    };
}
