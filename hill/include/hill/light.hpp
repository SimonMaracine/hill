#pragma once

#include "hill/glm.h++"

namespace hill::light {
    struct DirectionalLight {
        glm::vec3 direction {0.0f, 1.0f, 0.0f};
        glm::vec3 ambient_color {0.1f};
        glm::vec3 diffuse_color {0.5f};
        glm::vec3 specular_color {1.0f};
    };
}
