#pragma once

#include <memory>
#include <chrono>
#include <vector>

#include "hill/configuration.hpp"
#include "hill/imgui.hpp"
#include "hill/camera.hpp"
#include "hill/scene.hpp"
#include "hill/renderer_common.hpp"

namespace hill::editor {
    class Editor;
}

namespace hill::renderer {
    class Renderer {
    public:
        explicit Renderer(imgui::ImGui& imgui);
        Renderer(imgui::ImGui& imgui, const configuration::Configuration& configuration);

        void initialize();
        void uninitialize();
        void render();
        void window_resize(int width, int height);
    private:
        void imgui_initialize() const;
        void imgui_uninitialize() const;
        void imgui_render() const;

        void submit(std::shared_ptr<scene::ModelNode> node);

        void begin();
        void end();

        void draw_object(const renderer_common::Object& object) const;

        imgui::ImGui* m_imgui {};
        configuration::Configuration m_configuration;

        // std::shared_ptr<vertex_buffer::VertexBuffer> m_vertex_buffer;
        // std::shared_ptr<element_buffer::ElementBuffer> m_element_buffer;
        // std::shared_ptr<vertex_array::VertexArray> m_vertex_array;
        // std::shared_ptr<shader::Program> m_program;

        int m_window_width {};
        int m_window_height {};

        float m_background_color[3] { 0.4f, 0.1f, 0.6f };

        camera::Camera m_editor_camera;

        std::vector<renderer_common::Object> m_objects;

        std::chrono::high_resolution_clock::time_point m_last_time {};
        std::chrono::duration<double> m_frame_time {};

        friend class editor::Editor;
    };
}
