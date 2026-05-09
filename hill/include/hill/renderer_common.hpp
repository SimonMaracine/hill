#pragma once

#include <string>
#include <memory>

#include "hill/primitives/vertex_array.hpp"
#include "hill/material.hpp"
#include "hill/mesh.hpp"

namespace hill::renderer_common {
    struct Mesh {
        std::string name;
        std::shared_ptr<material::Material> material;
    };

    struct Object {
        int elements_count {};
        int elements_offset {};
        std::shared_ptr<vertex_array::VertexArray> vertex_array;
        std::shared_ptr<material::Material> material;
    };

    enum class ShaderSet {
        Basic
    };

    ShaderSet choose_shader_set(const mesh::Mesh& mesh);
}
