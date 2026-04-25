#pragma once

#include <glm/glm.hpp>

#include "hill/camera.hpp"

namespace hill::renderer {
    class Renderer;
}

namespace hill::editor {
    class Editor {
    public:
        void update(renderer::Renderer& renderer);
        void update_camera(renderer::Renderer& renderer);
    private:
        void performance(renderer::Renderer& renderer);

        void primitives_registry(renderer::Renderer& renderer);
        void primitives_object(const char* label, const auto& objects);

        struct Camera {
            glm::vec3 position {0.0f, 0.0f, 2.0f};
            glm::vec3 front {0.0f, 0.0f, -1.0f};
            glm::vec3 up {0.0f, 1.0f, 0.0f};

            float pitch {};
            float yaw = -90.0f;
        } m_camera;
    };
}
