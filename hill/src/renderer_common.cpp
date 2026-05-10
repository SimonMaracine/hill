#include "hill/renderer_common.hpp"

namespace hill::renderer_common {
    ShaderFeatureSet choose_shader_feature_set(const mesh::Mesh& mesh) {
        return ShaderFeatureBase;
    }
}
