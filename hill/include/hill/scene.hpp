#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <format>

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "hill/model.hpp"
#include "hill/mesh.hpp"
#include "hill/light.hpp"
#include "hill/material.hpp"
#include "hill/renderer_common.hpp"
#include "hill/editor_common.hpp"

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

        void add(std::shared_ptr<Node> child);
    protected:
        std::string m_name;
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

    class ModelNode : public Node {
    public:
        explicit ModelNode(std::string name)
            : Node(std::move(name)) {}

        void renderer_process(renderer::Renderer& renderer, renderer::TraversalCtx& ctx) override;
        void editor_inspect(editor::Editor& editor) override;
        void editor_nodes(editor::Editor& editor) override;

        static std::shared_ptr<ModelNode> from_model(const model::Model& model);

        std::size_t meshes_count() const { return m_runtime.objects.size(); }

        glm::vec3 position {};
        glm::vec3 rotation {};
        glm::vec3 scale {1.0f};
        std::unique_ptr<renderer_common::Mesh[]> meshes;
    private:
        struct TraversalCtx {
            std::weak_ptr<ModelNode> current_node;  // Used for propagation
        };

        static void traverse(TraversalCtx& ctx, const model::Node* node);
        static std::unique_ptr<renderer_common::Mesh[]> create_meshes(const std::vector<std::shared_ptr<mesh::Mesh>>& meshes);
        static std::shared_ptr<material::Material> create_material(const mesh::Mesh& mesh);

        struct {
            glm::mat4 transform = glm::identity<glm::mat4>();
            std::vector<std::shared_ptr<mesh::Mesh>> meshes;
        } m_static;

        struct {
            std::vector<renderer_common::Object> objects;
            bool configured {};
        } m_runtime;

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
}
