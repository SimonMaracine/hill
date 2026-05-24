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

        current_node->m_name = node->name;
        current_node->m_local.translation = node->translation;
        current_node->m_local.rotation = node->rotation;
        current_node->m_local.scale = node->scale;
        current_node->m_mesh_sources.append_range(node->meshes);
        current_node->m_meshes = create_meshes(node->meshes);

        for (const auto& child : node->children) {
            const auto child_node = std::make_shared<ModelNode>("");
            ctx.current_node = child_node;
            traverse(ctx, child.get());

            current_node->add(child_node);
        }
    }

    std::vector<Mesh> ModelNode::create_meshes(const std::vector<std::shared_ptr<mesh::MeshSource>>& mesh_sources) {
        std::vector<Mesh> result_meshes;
        result_meshes.reserve(mesh_sources.size());

        for (const auto& mesh : mesh_sources) {
            auto& [name, material, aabb] = result_meshes.emplace_back();
            name = mesh->name;
            material = create_material(*mesh);
            aabb = mesh->aabb;
        }

        return result_meshes;
    }

    std::shared_ptr<material::Material> ModelNode::create_material(const mesh::MeshSource& mesh_source) {
        material::MaterialDescription material_description;

        const auto shader_feature_set = renderer_common::choose_shader_feature_set(mesh_source);

        material_description.add_uniform("u_material.color_ambient", glm::vec3(0.0f));
        material_description.add_uniform("u_material.color_diffuse", glm::vec3(0.0f));
        material_description.add_uniform("u_material.color_specular", glm::vec3(0.0f));
        material_description.add_uniform("u_material.shininess", 0.0f);

        if (shader_feature_set & renderer_common::ShaderFeatureDiffuseMap) {
            material_description.remove_uniform("u_material.color_ambient");
            material_description.remove_uniform("u_material.color_diffuse");

            material_description.add_texture("u_material.texture_diffuse");
        }

        return std::make_shared<material::Material>(material_description);
    }

    void DirectionalLightNode::renderer_process(renderer::Renderer& renderer, renderer::TraversalCtx& ctx) {
        renderer.render_node(ctx, this);
    }

    void DirectionalLightNode::editor_inspect(editor::Editor& editor) {
        editor.inspect(this);
    }
}
