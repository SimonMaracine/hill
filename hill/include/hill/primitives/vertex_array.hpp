#pragma once

#include <memory>
#include <array>
#include <unordered_set>

#include "hill/primitives/common.hpp"
#include "hill/primitives/vertex_buffer.hpp"
#include "hill/primitives/element_buffer.hpp"

namespace hill::vertex_array {
    enum class Type {
        Float,
        Double,
        Int,
        UnsignedInt
    };

    struct Attribute {
        Attribute() = default;
        Attribute(unsigned int index, int components, Type type, bool normalized, int stride, unsigned int pointer)
            : index(index), components(components), type(type), normalized(normalized), stride(stride), pointer(pointer) {}

        unsigned int index {};
        int components {};
        Type type {};
        bool normalized {};
        int stride {};
        unsigned int pointer {};
    };

    class Layout {
    public:
        using const_pointer = const Attribute*;

        const_pointer begin() const { return m_attributes.data(); }
        const_pointer end() const { return m_attributes.data() + m_size; }
        std::size_t size() const { return m_size; }

        void attribute(const Attribute& attribute);
    private:
        std::array<Attribute, 8> m_attributes;
        std::size_t m_size {};
    };

    class VertexArray {
    public:
        VertexArray();
        ~VertexArray();

        VertexArray(const VertexArray&) = delete;
        VertexArray& operator=(const VertexArray&) = delete;
        VertexArray(VertexArray&&) = delete;
        VertexArray& operator=(VertexArray&&) = delete;

        unsigned int id() const { return m_vertex_array; }

        void bind() const;
        void unbind() const;
        void configure(std::shared_ptr<vertex_buffer::VertexBuffer> vertex_buffer, const Layout& layout);
        void configure_and_unbind(std::shared_ptr<element_buffer::ElementBuffer> element_buffer);
    private:
        unsigned int m_vertex_array {};
        std::unordered_set<std::shared_ptr<vertex_buffer::VertexBuffer>, common::IdHash<vertex_buffer::VertexBuffer>> m_vertex_buffers;
        std::shared_ptr<element_buffer::ElementBuffer> m_element_buffer;
    };
}
