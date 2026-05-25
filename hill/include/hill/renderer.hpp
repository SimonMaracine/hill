#pragma once

#include <memory>
#include <chrono>
#include <vector>
#include <unordered_map>

#include "hill/configuration.hpp"
#include "hill/imgui.hpp"
#include "hill/camera.hpp"
#include "hill/scene.hpp"
#include "hill/light.hpp"
#include "hill/material.hpp"
#include "hill/renderer_common.hpp"
#include "hill/aabb.hpp"
#include "hill/glm.h++"

namespace hill::editor {
    class Editor;
}

namespace hill::renderer {
    struct RenderObject : renderer_common::Object {
        glm::mat4 world_transform = glm::identity<glm::mat4>();
    };

    struct TraversalCtx {
        glm::mat4 parent_world_transform = glm::identity<glm::mat4>();
        bool dirty {};
    };

    class Renderer {
    public:
        explicit Renderer(imgui::ImGui& imgui, const configuration::Configuration& configuration = {});
        ~Renderer();

        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;
        Renderer(Renderer&&) = delete;
        Renderer& operator=(Renderer&&) = delete;

        void render();
        void window_resize(int width, int height);

        scene::RootNode* root_node() const { return m_root_node.get(); }

        void add_debug_line(glm::vec3 p1, glm::vec3 p2, glm::vec3 color = glm::vec3(1.0f));
        void add_debug_aabb(const aabb::Aabb& aabb, glm::vec3 color = glm::vec3(1.0f));
        void add_debug_aabb(const aabb::Aabb& aabb, const glm::mat4& transform, glm::vec3 color = glm::vec3(1.0f));
    private:
        void imgui_initialize() const;
        void imgui_uninitialize() const;
        void imgui_render() const;

        void debug_initialize();
        void debug_render();

        void render_initialize();
        void render_begin();
        void render_end();
        void render_traverse_tree(TraversalCtx& ctx, scene::Node* node);
        void render_node(TraversalCtx& ctx, scene::RootNode* node);
        void render_node(TraversalCtx& ctx, scene::ModelNode* node);
        void render_node(TraversalCtx& ctx, scene::DirectionalLightNode* node);

        void submit(const RenderObject& object);

        void draw_object(const RenderObject& object) const;

        void configure(scene::ModelNode* node);

        std::shared_ptr<vertex_array::VertexArray> create_vertex_array(const mesh::MeshSource& mesh_source) const;
        std::shared_ptr<shader::Program> create_program(renderer_common::ShaderFeatureSet shader_feature_set) const;
        std::shared_ptr<shader::Program> get_or_create_program(renderer_common::ShaderFeatureSet shader_feature_set);
        std::shared_ptr<material::Material> initialize_material(const mesh::Material& raw_material, std::shared_ptr<material::Material> material);
        std::shared_ptr<texture2d::Texture2D> get_or_create_texture(mesh::TextureSource texture_source);
        std::vector<std::string> create_vertex_shader_sources(renderer_common::ShaderFeatureSet shader_feature_set) const;
        std::vector<std::string> create_fragment_shader_sources(renderer_common::ShaderFeatureSet shader_feature_set) const;
        static void setup_shader_features(renderer_common::ShaderFeatureSet shader_feature_set, std::vector<std::string>& sources);

        // Client input
        imgui::ImGui* m_imgui {};
        configuration::Configuration m_configuration;

        // Static data
        std::string m_phong_vertex_shader_source;
        std::string m_phong_fragment_shader_source;

        // Runtime data
        int m_window_width {};
        int m_window_height {};
        glm::vec3 m_background_color {0.6f, 0.6f, 0.7f};

        camera::Camera m_camera;
        light::DirectionalLight m_directional_light;

        std::vector<RenderObject> m_objects;
        std::unordered_map<renderer_common::ShaderFeatureSet, std::weak_ptr<shader::Program>> m_programs;
        std::unordered_map<mesh::TextureSource, std::weak_ptr<texture2d::Texture2D>> m_textures;

        // Runtime scene hierarchy
        std::shared_ptr<scene::RootNode> m_root_node;

        // Debug renderer
        struct DebugRenderer {
            struct Line {
                glm::vec3 p1 {};
                glm::vec3 p2 {};
                glm::vec3 color {};
            };

            std::vector<Line> lines;

            std::shared_ptr<vertex_array::VertexArray> vertex_array;
            std::weak_ptr<vertex_buffer::VertexBuffer> weak_vertex_buffer;
            std::shared_ptr<shader::Program> program;
        } m_debug_renderer;

        // Performance data
        mutable struct {
            std::chrono::high_resolution_clock::time_point last_time {};
            std::chrono::duration<double> frame_time {};
            unsigned int draw_calls {};
            unsigned int transform_updates {};
        } m_performance;

        friend class editor::Editor;
        friend class scene::RootNode;
        friend class scene::ModelNode;
        friend class scene::DirectionalLightNode;
    };
}
