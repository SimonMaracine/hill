#include "hill/scene.hpp"

#include "hill/renderer.hpp"
#include "hill/editor.hpp"

namespace hill::scene {
    void Node::add(std::shared_ptr<Node> child) {
        const auto name = child->m_name;
        child->m_parent = weak_from_this();
        m_children[name] = std::move(child);
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

    std::shared_ptr<ModelNode> ModelNode::from_model(const model::Model& model) {
        TraversalCtx ctx;

        const auto result_node = std::make_shared<ModelNode>("");
        ctx.current_node = result_node;
        traverse(ctx, model.root());

        return result_node;
    }

    void ModelNode::traverse(TraversalCtx& ctx, const model::Node* node) {
        const auto current_node = ctx.current_node.lock();

        if (node->name.empty()) {
            throw error::Error("Invalid name");
        }

        current_node->translation = node->translation;
        current_node->rotation = glm::eulerAngles(node->rotation);
        current_node->scale = node->scale;
        current_node->m_name = node->name;
        current_node->m_meshes = create_meshes(node->meshes, current_node->m_meshes_count);
        current_node->m_static_meshes.append_range(node->meshes);

        for (const auto& child : node->children) {
            const auto child_node = std::make_shared<ModelNode>("");
            ctx.current_node = child_node;
            traverse(ctx, child.get());

            current_node->add(child_node);
        }
    }

    std::unique_ptr<renderer_common::Mesh[]> ModelNode::create_meshes(const std::vector<std::shared_ptr<mesh::Mesh>>& meshes, std::size_t& count) {
        count = meshes.size();
        auto result_meshes = std::make_unique<renderer_common::Mesh[]>(meshes.size());

        for (std::size_t i {}; const auto& mesh : meshes) {
            auto& [name, material] = result_meshes[i++];
            name = mesh->name;
            material = create_material(*mesh);
        }

        return result_meshes;
    }

    std::shared_ptr<material::Material> ModelNode::create_material(const mesh::Mesh& mesh) {
        std::shared_ptr<material::Material> result_material;

        switch (renderer_common::choose_shader_feature_set(mesh)) {
            case renderer_common::ShaderFeatureBase: {
                auto material = std::make_shared<material::MaterialBasic>();
                material->ambient_color = mesh.material.color_ambient;
                material->diffuse_color = mesh.material.color_diffuse;
                material->specular_color = mesh.material.color_specular;
                material->shininess = mesh.material.shininess;

                result_material = std::move(material);
            }
        }

        return result_material;
    }

    void DirectionalLightNode::renderer_process(renderer::Renderer& renderer, renderer::TraversalCtx& ctx) {
        renderer.render_node(ctx, this);
    }

    void DirectionalLightNode::editor_inspect(editor::Editor& editor) {
        editor.inspect(this);
    }
}
