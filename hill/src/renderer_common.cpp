#include "hill/renderer_common.hpp"

namespace hill::renderer_common {
    ShaderFeatureSet choose_shader_feature_set(const mesh::MeshSource& mesh_source) {
        ShaderFeatureSet shader_feature_set = ShaderFeatureBase;

        if (mesh_source.vertex_attributes & mesh::VertexAttributes::TextureCoordinates) {
            if (mesh_source.material.texture_diffuse) {
                shader_feature_set |= ShaderFeatureDiffuseMap;
            }

            if (mesh_source.material.texture_specular) {
                shader_feature_set |= ShaderFeatureSpecularMap;
            }
        }

        return shader_feature_set;
    }
}
