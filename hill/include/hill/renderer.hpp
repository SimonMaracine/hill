#pragma once

#include "hill/configuration.hpp"
#include "hill/imgui.hpp"

namespace hill::renderer {
    class Renderer {
    public:
        explicit Renderer(const imgui::ImGui& imgui);
        Renderer(const imgui::ImGui& imgui, const configuration::Configuration& configuration);
        ~Renderer();

        Renderer(const Renderer&) = delete;
        Renderer& operator=(const Renderer&) = delete;
        Renderer(Renderer&&) = delete;
        Renderer& operator=(Renderer&&) = delete;

        void initialize();
        void uninitialize();
        void render();
    private:
        void imgui_initialize() const;
        void imgui_uninitialize() const;
        void imgui_render();
        void imgui();

        const imgui::ImGui* m_imgui {};
        configuration::Configuration m_configuration;

        unsigned int m_vertex_buffer {};
        unsigned int m_index_buffer {};
        unsigned int m_vertex_array {};
        unsigned int m_shader_program {};
    };
}
