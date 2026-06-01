#pragma once

#include <memory>

#include "hill/primitives/texture_cubemap.hpp"
#include "hill/image.hpp"

namespace hill::renderer {
    class Renderer;
}

namespace hill::environment {
    struct SkyboxFaces {
        image::Image positive_x;
        image::Image negative_x;
        image::Image positive_y;
        image::Image negative_y;
        image::Image positive_z;
        image::Image negative_z;
    };

    class Skybox {
    public:
        Skybox() = default;
        explicit Skybox(std::shared_ptr<texture_cubemap::TextureCubemap> texture)
            : m_texture(std::move(texture)) {}
    private:
        std::shared_ptr<texture_cubemap::TextureCubemap> m_texture;

        friend class renderer::Renderer;
    };

    struct Fog {
        bool enabled {};
        float density {0.0075f};
        float gradient {3.0f};
        glm::vec3 color {0.6f};
    };

    struct Environment {
        Skybox skybox;
        Fog fog;
    };

    inline constexpr float SKYBOX_POSITIONS[] = {
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,

        -1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,

        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,

        -1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, 1.0f
    };
}
