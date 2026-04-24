#include "hill/primitives_registry.hpp"

#include <memory>

namespace hill::primitives_registry {
    static std::unique_ptr<Registry> g_instance;

    void Registry::initialize() {
        g_instance = std::make_unique<Registry>();
    }

    void Registry::uninitialize() {
        g_instance.reset();
    }

    Registry& Registry::get() {
        return *g_instance;
    }

    void Registry::add_vertex_buffer(unsigned int id) {
        m_vertex_buffers.insert(id);
    }

    void Registry::remove_vertex_buffer(unsigned int id) {
        m_vertex_buffers.erase(id);
    }

    void Registry::add_element_buffer(unsigned int id) {
        m_element_buffers.insert(id);
    }

    void Registry::remove_element_buffer(unsigned int id) {
        m_element_buffers.erase(id);
    }

    void Registry::add_vertex_array(unsigned int id) {
        m_vertex_arrays.insert(id);
    }

    void Registry::remove_vertex_array(unsigned int id) {
        m_vertex_arrays.erase(id);
    }

    void Registry::add_shader(unsigned int id) {
        m_shaders.insert(id);
    }

    void Registry::remove_shader(unsigned int id) {
        m_shaders.erase(id);
    }

    void Registry::add_program(unsigned int id) {
        m_programs.insert(id);
    }

    void Registry::remove_program(unsigned int id) {
        m_programs.erase(id);
    }
}
