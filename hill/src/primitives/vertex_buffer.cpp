#include "hill/primitives/vertex_buffer.hpp"

#include <glad/gl.h>

#include "hill/primitives_registry.hpp"

namespace hill::vertex_buffer {
    VertexBuffer::VertexBuffer() {
        glGenBuffers(1, &m_vertex_buffer);
        primitives_registry::add_primitive(primitives_registry::Primitive::VertexBuffer, m_vertex_buffer);
    }

    VertexBuffer::~VertexBuffer() {
        primitives_registry::remove_primitive(primitives_registry::Primitive::VertexBuffer, m_vertex_buffer);
        glDeleteBuffers(1, &m_vertex_buffer);
    }

    void VertexBuffer::bind() const {
        glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer);
    }

    void VertexBuffer::unbind() const {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void VertexBuffer::upload_data(const void* vertices, std::size_t size, common::BufferUsage usage) const {
        glBufferData(GL_ARRAY_BUFFER, long(size), vertices, common::buffer_usage_enum(usage));
    }
}
