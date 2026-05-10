#pragma once

#include <string>
#include <vector>
#include <filesystem>

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
        glm::vec2 uv {};
    };

    enum class TextureType {
        Albedo,
        Metallic,
        Normal
    };

    struct Texture {  // TODO image
        TextureType type {};
        std::filesystem::path path;
    };

    struct Material {
        std::string name;
        glm::vec3 color_ambient {0.6f};
        glm::vec3 color_diffuse {0.6f};
        glm::vec3 color_specular {0.6f};
        float shininess = 32.0f;
    };

    struct Mesh {
        std::string name;
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<Texture> textures;
        Material material;
        unsigned int vertex_attributes {};
    };
}
