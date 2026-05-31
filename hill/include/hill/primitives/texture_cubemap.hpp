#pragma once

namespace hill::texture_cubemap {
    union Data {
        struct {
            const void* positive_x;
            const void* negative_x;
            const void* positive_y;
            const void* negative_y;
            const void* positive_z;
            const void* negative_z;
        } structure;

        const void* array[6];
    };

    class TextureCubemap {
    public:
        TextureCubemap(int width, int height);
        ~TextureCubemap();

        TextureCubemap(const TextureCubemap&) = delete;
        TextureCubemap& operator=(const TextureCubemap&) = delete;
        TextureCubemap(TextureCubemap&&) = delete;
        TextureCubemap& operator=(TextureCubemap&&) = delete;

        unsigned int id() const { return m_texture; }
        int width() const { return m_width; }
        int height() const { return m_height; }

        void bind(unsigned int unit = 0) const;
        void unbind(unsigned int unit = 0) const;
        void upload_data(const Data& data) const;
    private:
        unsigned int m_texture {};
        int m_width {};
        int m_height {};
    };
}
