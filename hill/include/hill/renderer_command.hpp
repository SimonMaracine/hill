#pragma once

namespace hill::renderer_command {
    enum class Buffer {
        Color,
        Depth,
        Stencil
    };

    void clear(Buffer buffer);
    void draw_elements_triangles(int count, int offset = 0);
}
