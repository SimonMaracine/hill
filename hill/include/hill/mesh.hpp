#pragma once

#include <vector>
#include <filesystem>

#include <glm/glm.hpp>

namespace hill::mesh {
    enum VertexAttributes : unsigned int {
        Positions = 0,
        Colors = 1u << 0,
        Normals = 1u << 1,
        TextureCoordinates = 1u << 2
    };

    struct Vertex {
        glm::vec3 position {};
        glm::vec4 color {};
        glm::vec3 normal {};
        glm::vec2 texture_coordinate {};
    };

    enum class TextureType {
        Albedo,
        Specular,
        Normal
    };

    struct Texture {
        std::filesystem::path path;
        TextureType type {};
    };

    struct Mesh {
        unsigned int vertex_attributes {};
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<Texture> textures;
    };
}
