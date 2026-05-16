#pragma once

#include "hill/glm.h++"

namespace hill::aabb {
    struct Aabb {
        glm::vec3 min {};
        glm::vec3 max {};
    };
}
