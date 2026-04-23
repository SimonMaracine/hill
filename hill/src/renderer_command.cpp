#include "hill/renderer_command.hpp"

#include <utility>
#include <cstddef>
#include <cassert>

#include <glad/gl.h>

namespace hill::renderer_command {
    static constexpr unsigned int buffer_enum(Buffer buffer) {
        switch (buffer) {
            case Buffer::Color:
                return GL_COLOR_BUFFER_BIT;
            case Buffer::Depth:
                return GL_DEPTH_BUFFER_BIT;
            case Buffer::Stencil:
                return GL_STENCIL_BUFFER_BIT;
        }

        std::unreachable();
    }

    void clear(Buffer buffer) {
        // TODO
    }

    void draw_elements_triangles(int count, int offset) {
        assert(count > 0);
        assert(offset >= 0);

        glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, reinterpret_cast<void*>(std::size_t(offset) * sizeof(unsigned int)));
    }
}
