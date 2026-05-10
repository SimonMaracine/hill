#pragma once

#include "hill/glm.h++"

namespace hill::renderer_command {
    enum class Buffers {
        C, D, S, CD, CS, DS, CDS
    };

    void clear(Buffers buffers);
    void draw_elements_triangles(int count, int offset = 0);

    void clear_color(glm::vec4 color);

    void viewport(int x, int y, int width, int height);
    void viewport(int width, int height);

    void enable_depth_test();
    void disable_depth_test();
}
