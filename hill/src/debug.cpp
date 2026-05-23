#include "hill/debug.hpp"

#include <sstream>
#include <memory>
#include <utility>
#include <cassert>

#include <glad/gl.h>

namespace hill::debug {
    static Type severity_type(unsigned int severity) {
        switch (severity) {
            case GL_DEBUG_SEVERITY_HIGH: return Type::Error;
            case GL_DEBUG_SEVERITY_MEDIUM: return Type::Warning;
            case GL_DEBUG_SEVERITY_LOW: return Type::Information;
            case GL_DEBUG_SEVERITY_NOTIFICATION: return Type::Notification;
        }

        std::unreachable();
    }

    static void message_callback(
        unsigned int source,
        unsigned int type,
        unsigned int id,
        unsigned int severity,
        int,
        const char* message,
        const void* user_param
    ) {
        const auto debug_output = static_cast<const DebugOutput*>(user_param);

        if (!debug_output) {
            return;
        }

        std::ostringstream stream;

        stream << '[' << id << ']';
        stream << ' ';

        switch (severity) {
            case GL_DEBUG_SEVERITY_HIGH: stream << "[HIGH]"; break;
            case GL_DEBUG_SEVERITY_MEDIUM: stream << "[MEDIUM]"; break;
            case GL_DEBUG_SEVERITY_LOW: stream << "[LOW]"; break;
            case GL_DEBUG_SEVERITY_NOTIFICATION: stream << "[NOTIFICATION]"; break;
            default: assert(false); break;
        }

        stream << ' ';

        switch (source) {
            case GL_DEBUG_SOURCE_API: stream << "[API]"; break;
            case GL_DEBUG_SOURCE_WINDOW_SYSTEM: stream << "[Window System]"; break;
            case GL_DEBUG_SOURCE_SHADER_COMPILER: stream << "[Shader Compiler]"; break;
            case GL_DEBUG_SOURCE_THIRD_PARTY: stream << "[Third Party]"; break;
            case GL_DEBUG_SOURCE_APPLICATION: stream << "[Application]"; break;
            case GL_DEBUG_SOURCE_OTHER: stream << "[Other]"; break;
            default: assert(false); break;
        }

        stream << ' ';

        switch (type) {
            case GL_DEBUG_TYPE_ERROR: stream << "[Error]"; break;
            case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: stream << "[Deprecated Behavior]"; break;
            case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: stream << "[Undefined Behavior]"; break;
            case GL_DEBUG_TYPE_PORTABILITY: stream << "[Portability]"; break;
            case GL_DEBUG_TYPE_PERFORMANCE: stream << "[Performance]"; break;
            case GL_DEBUG_TYPE_MARKER: stream << "[Marker]"; break;
            case GL_DEBUG_TYPE_PUSH_GROUP: stream << "[Push Group]"; break;
            case GL_DEBUG_TYPE_POP_GROUP: stream << "[Pop Group]"; break;
            case GL_DEBUG_TYPE_OTHER: stream << "[Other]"; break;
            default: assert(false); break;
        }

        stream << ' ';
        stream << message;

        debug_output->output_callback(severity_type(severity), stream.str());
    }

    static std::unique_ptr<DebugOutput> g_debug_output;

    void initialize(DebugOutput debug_output) {
        g_debug_output = std::make_unique<DebugOutput>(std::move(debug_output));
    }

    void uninitialize() {
        g_debug_output.reset();
    }

    OutputCallback callback() {
        if (!g_debug_output) {
            return nullptr;
        }

        return g_debug_output->output_callback;
    }

    void enable_graphics_api() {
        if (!g_debug_output) {
            return;
        }

        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(message_callback, g_debug_output.get());

        if (g_debug_output->ignore_notification) {
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
        }
    }
}
