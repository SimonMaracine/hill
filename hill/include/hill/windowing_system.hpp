#pragma once

namespace hill::windowing_system {
    struct WindowingSystem {
        WindowingSystem() = default;
        virtual ~WindowingSystem() = default;

        WindowingSystem(const WindowingSystem&) = default;
        WindowingSystem& operator=(const WindowingSystem&) = default;
        WindowingSystem(WindowingSystem&&) = default;
        WindowingSystem& operator=(WindowingSystem&&) = default;

        virtual void grab_mouse() const = 0;
        virtual void ungrab_mouse() const = 0;
    };
}
