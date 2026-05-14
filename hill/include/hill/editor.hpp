#pragma once

#include <memory>
#include <string>

#include "hill/editor_common.hpp"
#include "hill/windowing_system.hpp"
#include "hill/material.hpp"
#include "hill/glm.h++"

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
    struct ModelMesh : editor_common::Inspectable {
        void editor_inspect(Editor& editor) override;

        std::shared_ptr<scene::ModelNode> node;
        std::size_t index {};
    };

    class Editor {
    public:
        void initialize();
        void uninitialize();
        void update(renderer::Renderer& renderer);
        void update_camera(renderer::Renderer& renderer, windowing_system::WindowingSystem& windowing_system);
    private:
        void performance(renderer::Renderer& renderer);

        void renderer(renderer::Renderer& renderer);

        void primitives_registry(renderer::Renderer& renderer);
        void primitives_object(const char* label, const auto& objects);

        void scene_hierarchy(renderer::Renderer& renderer);
        void scene_hierarchy_tree(scene::Node* node, std::string path);

        void inspector(renderer::Renderer& renderer);
        void inspect(scene::RootNode* node);
        void inspect(scene::ModelNode* node);
        void inspect(scene::DirectionalLightNode* node);
        void inspect(ModelMesh* mesh);

        void nodes(scene::ModelNode* node);

        bool material_basic(material::MaterialBasic* material);

        void gizmo(renderer::Renderer& renderer);

        void set_inspectable(std::shared_ptr<editor_common::Inspectable> inspectable, const std::string& name);
        static glm::mat4 ancestor_world_transform(std::shared_ptr<scene::Node> node);

        struct Camera {
            glm::vec3 position {0.0f, 0.0f, 20.0f};
            glm::vec3 front {0.0f, 0.0f, -1.0f};
            static constexpr glm::vec3 up {0.0f, 1.0f, 0.0f};

            float pitch {};
            float yaw = -90.0f;
            float move_speed_multiplier = 1.0f;
        } m_camera;

        struct Gizmo {
            int operation {};
            int mode {};
        } m_gizmo;

        char m_buffer_name[128] {};

        std::shared_ptr<editor_common::Inspectable> m_inspectable;

        friend class scene::RootNode;
        friend class scene::ModelNode;
        friend class scene::DirectionalLightNode;
        friend struct ModelMesh;
    };
}
