#pragma once

#include <memory>

#include "hill/primitives/vertex_array.hpp"

namespace hill::mesh {
    class Mesh {
    public:
        Mesh() = default;
        Mesh(int elements_count, std::shared_ptr<vertex_array::VertexArray> vertex_array)
            : m_elements_count(elements_count), m_vertex_array(std::move(vertex_array)) {}

        int elements_count() const { return m_elements_count; }
        std::shared_ptr<vertex_array::VertexArray> vertex_array() const { return m_vertex_array; }
    private:
        int m_elements_count {};
        std::shared_ptr<vertex_array::VertexArray> m_vertex_array;
    };
}
