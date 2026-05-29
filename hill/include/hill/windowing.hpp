#pragma once

namespace hill::windowing {
    struct Windowing {
        Windowing() = default;
        virtual ~Windowing() = default;

        Windowing(const Windowing&) = default;
        Windowing& operator=(const Windowing&) = default;
        Windowing(Windowing&&) = default;
        Windowing& operator=(Windowing&&) = default;

        virtual void windowing_grab_mouse() const = 0;
        virtual void windowing_ungrab_mouse() const = 0;
    };
}
