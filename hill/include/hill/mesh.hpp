#pragma once

#include <string>
#include <vector>
#include <memory>

#include "hill/aabb.hpp"
#include "hill/image.hpp"
#include "hill/glm.h++"

namespace hill::mesh {
    enum VertexAttributes : unsigned int {
        Positions = 0,
        Normals = 1u << 0,
        Colors = 1u << 1,
        TextureCoordinates = 1u << 2
    };

    struct Vertex {
        glm::vec3 position {};
        glm::vec3 normal {};
        glm::vec4 color {};
        glm::vec2 texture_coordinate {};
    };

    using TextureSource = std::shared_ptr<image::Image>;

    enum BlendMode : unsigned int {
        BlendModeNone,
        BlendModeDefault,
        BlendModeAdditive
    };

    struct Material {
        std::string name;

        glm::vec3 color_ambient {0.6f};
        glm::vec3 color_diffuse {0.6f};
        glm::vec3 color_specular {0.6f};
        float shininess = 32.0f;

        TextureSource texture_diffuse;
        TextureSource texture_specular;

        // Flags
        BlendMode blend_mode = BlendModeNone;
    };

    struct MeshSource {
        std::string name;
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        Material material;
        aabb::Aabb aabb;
        unsigned int vertex_attributes {};
    };
}
