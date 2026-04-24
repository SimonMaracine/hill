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

    void clear(Buffers buffers) {
        glClear(buffers_enum(buffers));
    }

    void draw_elements_triangles(int count, int offset) {
        assert(count > 0);
        assert(offset >= 0);

        glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, reinterpret_cast<void*>(std::size_t(offset) * sizeof(unsigned int)));
    }
    
    void clear_color(float r, float g, float b, float a) {
        glClearColor(r, g, b, a);
    }

    void viewport(int x, int y, int width, int height) {
        glViewport(x, y, width, height);
    }

    void viewport(int width, int height) {
        glViewport(0, 0, width, height);
    }
}
