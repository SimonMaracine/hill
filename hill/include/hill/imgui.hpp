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

        virtual void initialize() const = 0;
        virtual void uninitialize() const = 0;
        virtual void begin() const = 0;
        virtual void end(ImDrawData* draw_data) const = 0;
    };
}
