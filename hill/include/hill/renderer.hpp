#pragma once

#include <memory>

#include "hill/primitives/shader.hpp"
#include "hill/primitives/vertex_array.hpp"
#include "hill/primitives/vertex_buffer.hpp"
#include "hill/primitives/element_buffer.hpp"
#include "hill/configuration.hpp"
#include "hill/imgui.hpp"

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

        void submit(std::shared_ptr<shader::Program> program, std::shared_ptr<vertex_array::VertexArray> vertex_array, int count, int offset);

        imgui::ImGui* m_imgui {};
        configuration::Configuration m_configuration;

        std::shared_ptr<vertex_buffer::VertexBuffer> m_vertex_buffer;
        std::shared_ptr<element_buffer::ElementBuffer> m_element_buffer;
        std::shared_ptr<vertex_array::VertexArray> m_vertex_array;
        std::shared_ptr<shader::Program> m_program;

        int m_window_width {};
        int m_window_height {};

        float m_background_color[3] { 0.4f, 0.1f, 0.6f };

        double m_render_frame_time {};

        friend class editor::Editor;
    };
}
