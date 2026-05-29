#pragma once

#include <hill/renderer.hpp>
#include <hill/editor.hpp>

struct SDL_Window;

class SdlExample : public hill::imgui::ImGui, public hill::windowing::Windowing {
public:
    SdlExample();
    ~SdlExample() override;

    SdlExample(const SdlExample&) = delete;
    SdlExample& operator=(const SdlExample&) = delete;
    SdlExample(SdlExample&&) = delete;
    SdlExample& operator=(SdlExample&&) = delete;

    void imgui_initialize() const override;
    void imgui_uninitialize() const override;
    void imgui_begin() const override;
    void imgui_end(ImDrawData* draw_data) const override;
    void imgui_update() override;

    void windowing_grab_mouse() const override;
    void windowing_ungrab_mouse() const override;

    void run();
private:
    std::unique_ptr<hill::renderer::Renderer> m_renderer;
    std::unique_ptr<hill::editor::Editor> m_editor;

    SDL_Window* m_window {};
    void* m_context {};
    bool m_running = true;
};
