#pragma once

#include <hill/renderer.hpp>

struct SDL_Window;

class SdlExample : public hill::imgui::ImGui {
public:
    SdlExample();
    ~SdlExample() override;

    SdlExample(const SdlExample&) = delete;
    SdlExample& operator=(const SdlExample&) = delete;
    SdlExample(SdlExample&&) = delete;
    SdlExample& operator=(SdlExample&&) = delete;

    void initialize() const override;
    void uninitialize() const override;
    void begin() const override;
    void end(ImDrawData* draw_data) const override;

    void run();
private:
    hill::renderer::Renderer m_renderer;

    SDL_Window* m_window {};
    void* m_context {};
    bool m_running = true;
};
