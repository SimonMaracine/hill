#include "hill/renderer_command.hpp"

#include <utility>
#include <cstddef>
#include <cassert>

#include <glad/gl.h>

namespace hill::renderer_command {
    static constexpr unsigned int buffers_enum(Buffers buffers) {
        switch (buffers) {
            case Buffers::C:
                return GL_COLOR_BUFFER_BIT;
            case Buffers::D:
                return GL_DEPTH_BUFFER_BIT;
            case Buffers::S:
                return GL_STENCIL_BUFFER_BIT;
            case Buffers::CD:
                return GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT;
            case Buffers::CS:
                return GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;
            case Buffers::DS:
                return GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;
            case Buffers::CDS:
                return GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;
        }

        std::unreachable();
    }

    static constexpr unsigned int depth_function_enum(DepthFunction function) {
        switch (function) {
            case DepthFunction::Never:
                return GL_NEVER;
            case DepthFunction::Less:
                return GL_LESS;
            case DepthFunction::Equal:
                return GL_EQUAL;
            case DepthFunction::LEqual:
                return GL_LEQUAL;
            case DepthFunction::Greater:
                return GL_GREATER;
            case DepthFunction::NotEqual:
                return GL_NOTEQUAL;
            case DepthFunction::GEqual:
                return GL_GEQUAL;
            case DepthFunction::Always:
                return GL_ALWAYS;
        }

        std::unreachable();
    }

    void clear(Buffers buffers) {
        glClear(buffers_enum(buffers));
    }

    void draw_elements_triangles(int count, int offset) {
        assert(count > 0);
        assert(offset >= 0);

        glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, reinterpret_cast<void*>(std::size_t(offset) * sizeof(unsigned int)));
    }

    void draw_arrays_triangles(int count) {
        assert(count > 0);

        glDrawArrays(GL_TRIANGLES, 0, count);
    }

    void draw_arrays_lines(int count) {
        assert(count > 0);

        glDrawArrays(GL_LINES, 0, count);
    }

    void bind_texture_2d(unsigned int texture, unsigned int unit) {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, texture);
    }

    void unbind_texture_2d(unsigned int unit) {
        glActiveTexture(GL_TEXTURE0 + unit);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void clear_color(glm::vec4 color) {
        glClearColor(color.r, color.g, color.b, color.a);
    }

    void viewport(int x, int y, int width, int height) {
        glViewport(x, y, width, height);
    }

    void viewport(int width, int height) {
        glViewport(0, 0, width, height);
    }

    void depth_mask(bool flag) {
        glDepthMask(flag ? GL_TRUE : GL_FALSE);
    }

    void depth_function(DepthFunction function) {
        glDepthFunc(depth_function_enum(function));
    }

    void blend_mode(BlendMode mode) {
        switch (mode) {
            case BlendMode::Default:
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                break;
            case BlendMode::Additive:
                glBlendFunc(GL_ONE, GL_ONE);
                break;
        }
    }

    void enable_depth_test() {
        glEnable(GL_DEPTH_TEST);
    }

    void disable_depth_test() {
        glDisable(GL_DEPTH_TEST);
    }

    void enable_blend() {
        glEnable(GL_BLEND);
    }

    void disable_blend() {
        glDisable(GL_BLEND);
    }
}
