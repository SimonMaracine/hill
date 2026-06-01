#pragma once

#include "hill/glm.h++"

namespace hill::renderer_command {
    enum class Buffers {
        C, D, S, CD, CS, DS, CDS
    };

    enum class DepthFunction {
        Never,
        Less,
        Equal,
        LEqual,
        Greater,
        NotEqual,
        GEqual,
        Always
    };

    void clear(Buffers buffers);
    void draw_elements_triangles(int count, int offset = 0);
    void draw_arrays_triangles(int count);
    void draw_arrays_lines(int count);

    void bind_texture_2d(unsigned int texture, unsigned int unit);
    void unbind_texture_2d(unsigned int unit);

    void clear_color(glm::vec4 color);

    void viewport(int x, int y, int width, int height);
    void viewport(int width, int height);

    void depth_mask(bool flag);
    void depth_function(DepthFunction function);

    void enable_depth_test();
    void disable_depth_test();
}
