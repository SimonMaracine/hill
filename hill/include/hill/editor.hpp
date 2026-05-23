#pragma once

#include <memory>
#include <string>

#include "hill/editor_common.hpp"
#include "hill/windowing_system.hpp"
#include "hill/material.hpp"
#include "hill/aabb.hpp"
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
        explicit Editor(windowing_system::WindowingSystem& windowing_system);
        ~Editor();

        Editor(const Editor&) = delete;
        Editor& operator=(const Editor&) = delete;
        Editor(Editor&&) = delete;
        Editor& operator=(Editor&&) = delete;

        void update(renderer::Renderer& renderer);
        void update_camera(renderer::Renderer& renderer);
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

        void world_origin(renderer::Renderer& renderer);
        void world_grid(renderer::Renderer& renderer);
        void world_bounding_box(renderer::Renderer& renderer);

        bool bounding_box(renderer::Renderer& renderer, ModelMesh* mesh);
        bool bounding_box(renderer::Renderer& renderer, scene::ModelNode* node);
        void bounding_box(scene::ModelNode* node, aabb::Aabb& aabb);

        void set_inspectable(std::shared_ptr<editor_common::Inspectable> inspectable, const std::string& name);
        static glm::mat4 ancestor_world_transform(std::shared_ptr<scene::Node> node);

        windowing_system::WindowingSystem* m_windowing_system {};

        struct Camera {
            static constexpr glm::vec3 UP {0.0f, 1.0f, 0.0f};
            static constexpr glm::vec3 POSITION {0.0f, 3.0f, 30.0f};
            static constexpr glm::vec3 FRONT {0.0f, 0.0f, -1.0f};
            static constexpr float PITCH {};
            static constexpr float YAW = -90.0f;

            glm::vec3 position = POSITION;
            glm::vec3 front = FRONT;
            float pitch = PITCH;
            float yaw = YAW;
            float move_speed_multiplier = 1.0f;
            bool control {};
        } m_camera;

        char m_buffer_name[128] {};

        std::shared_ptr<editor_common::Inspectable> m_inspectable;

        friend class scene::RootNode;
        friend class scene::ModelNode;
        friend class scene::DirectionalLightNode;
        friend struct ModelMesh;
    };
}
