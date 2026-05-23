#pragma once

#include <string>

namespace hill::debug {
    enum class Type {
        Notification,
        Debug,
        Information,
        Warning,
        Error,
        Critical
    };

    using OutputCallback = void(*)(Type, std::string);

    struct DebugOutput {
        OutputCallback output_callback {};
        bool ignore_notification = true;
    };

    void initialize(DebugOutput debug_output);
    void uninitialize();

    OutputCallback callback();
    void enable_graphics_api();
}
