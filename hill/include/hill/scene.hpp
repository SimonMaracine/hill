#pragma once

#include <vector>
#include <unordered_map>
#include <string>

#include "hill/model.hpp"
#include "hill/mesh.hpp"
#include "hill/light.hpp"
#include "hill/material.hpp"
#include "hill/renderer_common.hpp"
#include "hill/editor_common.hpp"
#include "hill/glm.h++"

namespace hill::renderer {
    struct TraversalCtx;
    class Renderer;
}

namespace hill::editor {
    class Editor;
}

namespace hill::scene {
    class Node : public editor_common::Inspectable, public std::enable_shared_from_this<Node> {
    public:
        Node() = default;
        explicit Node(std::string name)
            : m_name(std::move(name)) {}

        Node(const Node&) = delete;
        Node& operator=(const Node&) = delete;
        Node(Node&&) = delete;
        Node& operator=(Node&&) = delete;

        virtual void renderer_process(renderer::Renderer& renderer, renderer::TraversalCtx& ctx) = 0;
        virtual void editor_nodes(editor::Editor&) {}

        const std::string& name() const { return m_name; }

        void child(std::shared_ptr<Node> child);
        void parent(std::shared_ptr<Node> parent);  // TODO test
        std::shared_ptr<Node> parent() const;
        void detach();
    protected:
        std::string m_name;
        std::weak_ptr<Node> m_parent;
        std::unordered_map<std::string, std::shared_ptr<Node>> m_children;

        friend class renderer::Renderer;
        friend class editor::Editor;
    };

    class RootNode : public Node {
    public:
        void renderer_process(renderer::Renderer& renderer, renderer::TraversalCtx& ctx) override;
        void editor_inspect(editor::Editor& editor) override;
    private:
        friend class renderer::Renderer;
    };

    struct Mesh {
        std::string name;
        std::shared_ptr<material::Material> material;
        aabb::Aabb aabb;
    };

    class ModelNode : public Node {
    public:
        explicit ModelNode(std::string name)
            : Node(std::move(name)) {}

        void renderer_process(renderer::Renderer& renderer, renderer::TraversalCtx& ctx) override;
        void editor_inspect(editor::Editor& editor) override;
        void editor_nodes(editor::Editor& editor) override;

        static std::shared_ptr<ModelNode> from_model(const model::Model& model);

        const Mesh* meshes() const { return m_meshes.data(); }
        std::size_t meshes_count() const { return m_meshes.size(); }

        glm::vec3 translation() const { return m_local.translation; }
        glm::vec3 rotation() const { return glm::degrees(glm::eulerAngles(m_local.rotation)); }
        glm::vec3 scale() const { return m_local.scale; }

        void translation(glm::vec3 translation) { m_local.translation = translation; m_world_transform_dirty = true; }
        void rotation(glm::vec3 rotation) { m_local.rotation = glm::quat(glm::radians(rotation)); m_world_transform_dirty = true; }
        void scale(glm::vec3 scale) { m_local.scale = scale; m_world_transform_dirty = true; }
    private:
        struct TraversalCtx {
            std::weak_ptr<ModelNode> current_node;  // Used for propagation
        };

        static void traverse(TraversalCtx& ctx, const model::Node* node);
        static std::vector<Mesh> create_meshes(const std::vector<std::shared_ptr<mesh::MeshSource>>& mesh_sources);
        static std::shared_ptr<material::Material> create_material(const mesh::MeshSource& mesh_source);

        struct {
            glm::vec3 translation {};
            glm::quat rotation = glm::identity<glm::quat>();
            glm::vec3 scale {1.0f};
        } m_local;

        std::vector<std::shared_ptr<mesh::MeshSource>> m_mesh_sources;  // Static
        std::vector<Mesh> m_meshes;
        std::vector<renderer_common::Object> m_render_objects;
        glm::mat4 m_world_transform = glm::identity<glm::mat4>();
        bool m_render_objects_dirty = true;
        bool m_world_transform_dirty = true;
        bool m_program_dirty = true;

        friend class renderer::Renderer;
        friend class editor::Editor;
    };

    class DirectionalLightNode : public Node {
    public:
        explicit DirectionalLightNode(std::string name, const light::DirectionalLight& directional_light = {})
            : Node(std::move(name)), directional_light(directional_light) {}

        void renderer_process(renderer::Renderer& renderer, renderer::TraversalCtx& ctx) override;
        void editor_inspect(editor::Editor& editor) override;

        light::DirectionalLight directional_light;
    private:
        friend class renderer::Renderer;
    };

    class PointLightNode : public Node {
    public:
        explicit PointLightNode(std::string name, const light::PointLight& point_light = {})
            : Node(std::move(name)), point_light(point_light) {}

        void renderer_process(renderer::Renderer& renderer, renderer::TraversalCtx& ctx) override;
        void editor_inspect(editor::Editor& editor) override;

        light::PointLight point_light;
    private:
        friend class renderer::Renderer;
    };

    class SpotLightNode : public Node {
    public:
        explicit SpotLightNode(std::string name, const light::SpotLight& spot_light = {})
            : Node(std::move(name)), spot_light(spot_light) {}

        void renderer_process(renderer::Renderer& renderer, renderer::TraversalCtx& ctx) override;
        void editor_inspect(editor::Editor& editor) override;

        light::SpotLight spot_light;
    private:
        friend class renderer::Renderer;
    };
}
