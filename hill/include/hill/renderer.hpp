#pragma once

#include <memory>
#include <chrono>
#include <vector>
#include <unordered_map>
#include <optional>

#include "hill/configuration.hpp"
#include "hill/imgui.hpp"
#include "hill/camera.hpp"
#include "hill/scene.hpp"
#include "hill/light.hpp"
#include "hill/environment.hpp"
#include "hill/material.hpp"
#include "hill/renderer_common.hpp"
#include "hill/aabb.hpp"
#include "hill/task_manager.hpp"
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
        void skybox(environment::Skybox skybox);
        environment::Skybox skybox(const environment::SkyboxFaces& skybox_faces);

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

        void skybox_initialize();
        void skybox_render() const;

        void render_initialize();
        void render_begin();
        void render_end();
        void render_traverse_tree(TraversalCtx& ctx, scene::Node* node);
        void render_node(TraversalCtx& ctx, scene::RootNode* node);
        void render_node(TraversalCtx& ctx, scene::ModelNode* node);
        void render_node(TraversalCtx& ctx, scene::DirectionalLightNode* node);
        void render_node(TraversalCtx& ctx, scene::PointLightNode* node);
        void render_node(TraversalCtx& ctx, scene::SpotLightNode* node);

        void submit(const RenderObject& object);
        void draw_object(const RenderObject& object) const;

        void configure(scene::ModelNode* node);
        void upload_program_shared_uniform_data();

        std::shared_ptr<vertex_array::VertexArray> create_vertex_array(const mesh::MeshSource& mesh_source) const;
        std::shared_ptr<shader::Program> create_program(renderer_common::ShaderFeatureSet shader_feature_set) const;
        std::shared_ptr<shader::Program> get_or_create_program(renderer_common::ShaderFeatureSet shader_feature_set);
        std::shared_ptr<material::Material> initialize_material(const mesh::Material& raw_material, std::shared_ptr<material::Material> material);
        std::shared_ptr<texture_2d::Texture2D> get_or_create_texture(mesh::TextureSource texture_source);
        std::vector<std::string> create_vertex_shader_sources(renderer_common::ShaderFeatureSet shader_feature_set) const;
        std::vector<std::string> create_fragment_shader_sources(renderer_common::ShaderFeatureSet shader_feature_set) const;
        static void setup_shader_features(renderer_common::ShaderFeatureSet shader_feature_set, std::vector<std::string>& sources);

        // Utility systems
        task_manager::TaskManager m_task_manager;  // Updated at the end of every frame

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
        environment::Skybox m_skybox;
        camera::Camera m_camera;

        // Runtime scene hierarchy
        std::shared_ptr<scene::RootNode> m_root_node;

        struct DebugRenderer {
            struct Line {
                glm::vec3 p1 {};
                glm::vec3 p2 {};
                glm::vec3 color {};
            };

            std::vector<Line> lines;

            std::shared_ptr<vertex_array::VertexArray> vertex_array;
            std::weak_ptr<vertex_buffer::VertexBuffer> w_vertex_buffer;
            std::shared_ptr<shader::Program> program;
        } m_debug_renderer;

        struct SkyboxRenderer {
            std::weak_ptr<texture_cubemap::TextureCubemap> w_texture;
            std::shared_ptr<vertex_array::VertexArray> vertex_array;
            std::shared_ptr<shader::Program> program;
        } m_skybox_renderer;

        struct MainRenderer {
            std::optional<light::DirectionalLight> directional_light;
            std::vector<light::PointLight> point_lights;
            std::vector<light::SpotLight> spot_lights;

            std::vector<RenderObject> objects;
            std::unordered_map<renderer_common::ShaderFeatureSet, std::weak_ptr<shader::Program>> programs;
            std::unordered_map<mesh::TextureSource, std::weak_ptr<texture_2d::Texture2D>> textures;
        } m_main_renderer;

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
        friend class scene::PointLightNode;
        friend class scene::SpotLightNode;
    };
}
