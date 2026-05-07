#pragma once

#include <memory>
#include <string>

#include <glm/glm.hpp>

#include "material.hpp"

namespace hill::renderer {
    class Renderer;
}

namespace hill::scene {
    class Node;
    class RootNode;
    class ModelNode;
    class DirectionalLightNode;
}

namespace hill::editor {
    class Editor {
    public:
        void initialize();
        void update(renderer::Renderer& renderer);
        void update_camera(renderer::Renderer& renderer);
    private:
        void performance(renderer::Renderer& renderer);

        void primitives_registry(renderer::Renderer& renderer);
        void primitives_object(const char* label, const auto& objects);

        void scene_hierarchy(renderer::Renderer& renderer);
        void scene_hierarchy_tree(scene::Node* node, std::string path);

        void node_properties(renderer::Renderer& renderer);
        void node_properties(scene::RootNode* node);
        void node_properties(scene::ModelNode* node);
        void node_properties(scene::DirectionalLightNode* node);

        bool material_basic(material::MaterialBasic* material);

        struct Camera {
            glm::vec3 position {0.0f, 0.0f, 20.0f};
            glm::vec3 front {0.0f, 0.0f, -1.0f};
            glm::vec3 up {0.0f, 1.0f, 0.0f};

            float pitch {};
            float yaw = -90.0f;
            float move_speed_multiplier = 1.0f;
        } m_camera;

        std::weak_ptr<scene::Node> m_wselected_node;

        friend class scene::RootNode;
        friend class scene::ModelNode;
        friend class scene::DirectionalLightNode;
    };
}
