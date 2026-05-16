#include "hill/primitives/element_buffer.hpp"

#include <glad/gl.h>

#include "hill/primitives_registry.hpp"

namespace hill::element_buffer {
    ElementBuffer::ElementBuffer() {
        glGenBuffers(1, &m_element_buffer);
        primitives_registry::add_primitive(primitives_registry::Primitive::ElementBuffer, m_element_buffer);
    }

    ElementBuffer::~ElementBuffer() {
        primitives_registry::remove_primitive(primitives_registry::Primitive::ElementBuffer, m_element_buffer);
        glDeleteBuffers(1, &m_element_buffer);
    }

    void ElementBuffer::bind() const {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_element_buffer);
    }

    void ElementBuffer::unbind() const {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    void ElementBuffer::upload_data(const void* indices, std::size_t size, common::BufferUsage usage) const {
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, long(size), indices, common::buffer_usage_enum(usage));
    }
}
