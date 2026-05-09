#include "hill/renderer_common.hpp"

namespace hill::renderer_common {
    ShaderSet choose_shader_set(const mesh::Mesh& mesh) {
        return ShaderSet::Basic;
    }
}
