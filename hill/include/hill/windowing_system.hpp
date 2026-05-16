#pragma once

namespace hill::windowing_system {
    struct WindowingSystem {
        WindowingSystem() = default;
        virtual ~WindowingSystem() = default;

        WindowingSystem(const WindowingSystem&) = default;
        WindowingSystem& operator=(const WindowingSystem&) = default;
        WindowingSystem(WindowingSystem&&) = default;
        WindowingSystem& operator=(WindowingSystem&&) = default;

        virtual void windowing_system_grab_mouse() const = 0;
        virtual void windowing_system_ungrab_mouse() const = 0;
    };
}
