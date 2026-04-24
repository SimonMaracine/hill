#include "hill/primitives/vertex_array.hpp"

#include <utility>

#include <glad/gl.h>

#include "hill/primitives_registry.hpp"

namespace hill::vertex_array {
    static constexpr unsigned int type_enum(Type type) {
        switch (type) {
            case Type::Float:
                return GL_FLOAT;
            case Type::Double:
                return GL_DOUBLE;
            case Type::Int:
                return GL_INT;
            case Type::UnsignedInt:
                return GL_UNSIGNED_INT;
        }

        std::unreachable();
    }

    void Layout::attribute(const Attribute& attribute) {
        m_attributes[m_size] = attribute;
        m_size++;
    }

    VertexArray::VertexArray() {
        glGenVertexArrays(1, &m_vertex_array);
        primitives_registry::Registry::get().add_vertex_array(m_vertex_array);
    }

    VertexArray::~VertexArray() {
        primitives_registry::Registry::get().remove_vertex_array(m_vertex_array);
        glDeleteVertexArrays(1, &m_vertex_array);
    }

    void VertexArray::bind() const {
        glBindVertexArray(m_vertex_array);
    }

    void VertexArray::unbind() const {
        glBindVertexArray(0);
    }

    void VertexArray::configure(std::shared_ptr<vertex_buffer::VertexBuffer> vertex_buffer, const Layout& layout) {
        vertex_buffer->bind();

        for (auto attribute = layout.begin(); attribute != layout.end(); attribute++) {
            glVertexAttribPointer(
                attribute->index,
                attribute->components,
                type_enum(attribute->type),
                attribute->normalized,
                attribute->stride,
                reinterpret_cast<void*>(attribute->pointer)
            );

            glEnableVertexAttribArray(attribute->index);
        }

        vertex_buffer->unbind();
        m_vertex_buffers.insert(std::move(vertex_buffer));
    }

    void VertexArray::configure_and_unbind(std::shared_ptr<element_buffer::ElementBuffer> element_buffer) {
        m_element_buffer = std::move(element_buffer);
        m_element_buffer->bind();
        unbind();
        m_element_buffer->unbind();
    }
}
