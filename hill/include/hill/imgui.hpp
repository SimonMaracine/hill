#pragma once

struct ImDrawData;

namespace hill::imgui {
    struct ImGui {
        ImGui() = default;
        virtual ~ImGui() = default;

        ImGui(const ImGui&) = default;
        ImGui& operator=(const ImGui&) = default;
        ImGui(ImGui&&) = default;
        ImGui& operator=(ImGui&&) = default;

        virtual void imgui_initialize() const = 0;
        virtual void imgui_uninitialize() const = 0;
        virtual void imgui_begin() const = 0;
        virtual void imgui_end(ImDrawData* draw_data) const = 0;
        virtual void imgui_update() = 0;
    };
}
