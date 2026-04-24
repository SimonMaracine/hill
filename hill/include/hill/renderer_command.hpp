#pragma once

namespace hill::renderer_command {
    enum class Buffers {
        C, D, S, CD, CS, DS, CDS
    };

    void clear(Buffers buffers);
    void draw_elements_triangles(int count, int offset = 0);

    void clear_color(float r, float g, float b, float a);

    void viewport(int x, int y, int width, int height);
    void viewport(int width, int height);
}
