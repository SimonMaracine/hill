#pragma once

#include <memory>

#include "hill/primitives/vertex_array.hpp"
#include "hill/material.hpp"
#include "hill/mesh.hpp"

namespace hill::renderer_common {
    struct Object {
        int elements_count {};
        int elements_offset {};
        std::shared_ptr<vertex_array::VertexArray> vertex_array;
        std::shared_ptr<material::Material> material;
    };

    enum ShaderFeature : unsigned int {
        ShaderFeatureBase = 0,
        ShaderFeatureVertexColors = 1u << 0,
        ShaderFeatureDiffuseMap = 1u << 1,
        ShaderFeatureSpecularMap = 1u << 2,
        ShaderFeatureNormalMap = 1u << 3,
        ShaderFeatureEmissionMap = 1u << 4,
        ShaderFeatureFog = 1u << 5,
        ShaderFeatureShadow = 1u << 6
    };

    using ShaderFeatureSet = unsigned int;

    ShaderFeatureSet choose_shader_feature_set(const mesh::Mesh& mesh);
}
