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

    void RootNode::editor_process(editor::Editor& editor) {
        editor.node_properties(this);
    }

    void ModelNode::renderer_process(renderer::Renderer& renderer, renderer::TraversalCtx& ctx) {
        renderer.render_node(ctx, this);
    }

    void ModelNode::editor_process(editor::Editor& editor) {
        editor.node_properties(this);
    }

    std::shared_ptr<ModelNode> ModelNode::create(std::string name, std::shared_ptr<model::Model> model) {
        // TODO
        return std::make_shared<ModelNode>(std::move(name));
    }

    void DirectionalLightNode::renderer_process(renderer::Renderer& renderer, renderer::TraversalCtx& ctx) {
        renderer.render_node(ctx, this);
    }

    void DirectionalLightNode::editor_process(editor::Editor& editor) {
        editor.node_properties(this);
    }
}
