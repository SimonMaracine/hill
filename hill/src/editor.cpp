#include "hill/editor.hpp"

#include <ranges>

#include <imgui.h>

#include "hill/renderer.hpp"
#include "hill/primitives_registry.hpp"

namespace hill::editor {
    void Editor::update(renderer::Renderer& renderer) {
        ImGui::ShowDemoWindow();

        ImGui::Begin("Test");
        ImGui::ColorEdit3("Background Color", renderer.m_background_color);
        ImGui::End();

        performance(renderer);
        primitives_registry(renderer);
    }

    void Editor::performance(renderer::Renderer& renderer) {
        if (ImGui::Begin("Performance")) {
            ImGui::Text("Frame time:");
        }

        ImGui::End();
    }

    void Editor::primitives_registry(renderer::Renderer&) {
        if (ImGui::Begin("Primitives Registry")) {
            primitives_object("Vertex Buffers", primitives_registry::Registry::get().vertex_buffers());
            primitives_object("Element Buffers", primitives_registry::Registry::get().element_buffers());
            primitives_object("Vertex Arrays", primitives_registry::Registry::get().vertex_arrays());
            primitives_object("Shaders", primitives_registry::Registry::get().shaders());
            primitives_object("Programs", primitives_registry::Registry::get().programs());
        }

        ImGui::End();
    }

    void Editor::primitives_object(const char* label, const auto& objects) {
        if (ImGui::TreeNode(label)) {
            for (const auto [i, object] : objects | std::views::enumerate) {
                ImGui::Text("%ld. %u", i + 1, object);
            }

            ImGui::TreePop();
        }
    }
}
