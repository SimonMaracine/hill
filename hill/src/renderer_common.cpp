#include "hill/renderer_common.hpp"

namespace hill::renderer_common {
    ShaderFeatureSet choose_shader_feature_set(const mesh::Mesh& mesh) {
        ShaderFeatureSet shader_feature_set = ShaderFeatureBase;

        if (mesh.vertex_attributes & mesh::VertexAttributes::TextureCoordinates) {
            if (mesh.material.texture_diffuse) {
                shader_feature_set |= ShaderFeatureDiffuseMap;
            }
        }

        return shader_feature_set;
    }
}
