#include "hill/renderer.hpp"

#include <glad/gl.h>  // FIXME remove
#include <imgui.h>

namespace hill::renderer {
    Renderer::Renderer(const imgui::ImGui& imgui)
        : m_imgui(&imgui) {}

    Renderer::Renderer(const imgui::ImGui& imgui, const configuration::Configuration& configuration)
        : m_imgui(&imgui), m_configuration(configuration) {}

    Renderer::~Renderer() {

    }

    void Renderer::initialize() {
        imgui_initialize();

        glClearColor(0.4f, 0.1f, 0.6f, 1.0f);
    }

    void Renderer::uninitialize() {
        imgui_uninitialize();
    }

    void Renderer::render() {
        glClear(GL_COLOR_BUFFER_BIT);

        imgui_render();
    }

    void Renderer::imgui_initialize() const {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        m_imgui->initialize();
    }

    void Renderer::imgui_uninitialize() const {
        m_imgui->uninitialize();
        ImGui::DestroyContext();
    }

    void Renderer::imgui_render() {
        m_imgui->begin();
        ImGui::NewFrame();

        imgui();

        ImGui::EndFrame();
        ImGui::Render();
        m_imgui->end(ImGui::GetDrawData());
    }

    void Renderer::imgui() {
        ImGui::ShowDemoWindow();
    }
}
