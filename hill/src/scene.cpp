#include "hill/scene.hpp"

#include "hill/renderer.hpp"
#include "hill/editor.hpp"

namespace hill::scene {
    void Node::add(std::shared_ptr<Node> child) {
        const auto name = child->name();
        m_children[std::string(name)] = std::move(child);
    }

    void RootNode::renderer_process(renderer::Renderer& renderer, renderer::TraversalCtx& ctx) {
        renderer.render_node(ctx, this);
    }

    void RootNode::editor_inspect(editor::Editor& editor) {
        editor.inspect(this);
    }

    void ModelNode::renderer_process(renderer::Renderer& renderer, renderer::TraversalCtx& ctx) {
        renderer.render_node(ctx, this);
    }

    void ModelNode::editor_inspect(editor::Editor& editor) {
        editor.inspect(this);
    }

    void ModelNode::editor_nodes(editor::Editor& editor) {
        editor.nodes(this);
    }

    std::shared_ptr<ModelNode> ModelNode::from_model(std::shared_ptr<model::Model> model) {
        TraversalCtx ctx;
        ctx.model = model;

        const auto result_node = std::make_shared<ModelNode>("");
        ctx.current_node = result_node;
        traverse(ctx, model->root());

        return result_node;
    }

    void ModelNode::traverse(TraversalCtx& ctx, const model::Node* model_node) {
        const auto current_node = ctx.current_node.lock();

        if (model_node->name.empty()) {
            throw error::Error("Invalid name");
        }

        current_node->m_name = model_node->name;
        current_node->m_transform = model_node->transform;
        current_node->m_model = ctx.model;
        current_node->m_meshes.append_range(model_node->meshes);
        current_node->m_objects.resize(current_node->m_meshes.size());

        for (const auto& child : model_node->children) {
            const auto child_node = std::make_shared<ModelNode>("");
            ctx.current_node = child_node;
            traverse(ctx, child.get());

            current_node->add(child_node);
        }
    }

    void DirectionalLightNode::renderer_process(renderer::Renderer& renderer, renderer::TraversalCtx& ctx) {
        renderer.render_node(ctx, this);
    }

    void DirectionalLightNode::editor_inspect(editor::Editor& editor) {
        editor.inspect(this);
    }
}
