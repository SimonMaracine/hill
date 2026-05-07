#pragma once

#include <memory>
#include <chrono>
#include <vector>
#include <unordered_map>

#include <glm/ext/matrix_transform.hpp>

#include "hill/configuration.hpp"
#include "hill/imgui.hpp"
#include "hill/camera.hpp"
#include "hill/scene.hpp"
#include "hill/light.hpp"
#include "hill/material.hpp"
#include "hill/renderer_common.hpp"

namespace hill::editor {
    class Editor;
}

namespace hill::renderer {
    enum class ShaderSet {
        Basic
    };

    struct RenderObject : renderer_common::Object {
        glm::mat4 transform = glm::identity<glm::mat4>();
    };

    struct TraversalCtx {
        glm::mat4 transform = glm::identity<glm::mat4>();
    };

    class Renderer {
    public:
        explicit Renderer(imgui::ImGui& imgui);
        Renderer(imgui::ImGui& imgui, const configuration::Configuration& configuration);

        void initialize();
        void uninitialize();
        void render();
        void window_resize(int width, int height);

        scene::RootNode* root_node() const { return m_root_node.get(); }
    private:
        void imgui_initialize() const;
        void imgui_uninitialize() const;
        void imgui_render() const;

        void submit(const RenderObject& object);

        void render_begin();
        void render_end();
        void render_traverse_tree(TraversalCtx& ctx, scene::Node* node);
        void render_node(TraversalCtx& ctx, scene::RootNode* node);
        void render_node(TraversalCtx& ctx, scene::ModelNode* node);
        void render_node(TraversalCtx& ctx, scene::DirectionalLightNode* node);

        void draw_object(const RenderObject& object) const;

        void configure(scene::ModelNode* node);

        std::shared_ptr<vertex_array::VertexArray> create_vertex_array(const mesh::Mesh& mesh) const;
        std::shared_ptr<material::Material> create_material(const mesh::Mesh& mesh, const model::Model& model);
        std::shared_ptr<shader::Program> create_program(ShaderSet shader_set);

        static ShaderSet choose_shader_set(const mesh::Mesh& mesh, const model::Model& model);

        imgui::ImGui* m_imgui {};
        configuration::Configuration m_configuration;

        int m_window_width {};
        int m_window_height {};

        float m_background_color[3] { 0.4f, 0.1f, 0.6f };

        camera::Camera m_camera;
        light::DirectionalLight m_directional_light;

        std::shared_ptr<scene::RootNode> m_root_node;

        std::vector<RenderObject> m_objects;
        std::unordered_map<ShaderSet, std::weak_ptr<shader::Program>> m_programs;

        std::chrono::high_resolution_clock::time_point m_last_time {};
        std::chrono::duration<double> m_frame_time {};

        friend class editor::Editor;
        friend class scene::RootNode;
        friend class scene::ModelNode;
        friend class scene::DirectionalLightNode;
    };
}
