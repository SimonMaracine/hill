#pragma once

#include <unordered_set>

namespace hill::primitives_registry {
    class Registry {
    public:
        static void initialize();
        static void uninitialize();
        static Registry& get();

        using Container = std::unordered_set<unsigned int>;

        const Container& vertex_buffers() const { return m_vertex_buffers; }
        const Container& element_buffers() const { return m_element_buffers; }
        const Container& vertex_arrays() const { return m_vertex_arrays; }
        const Container& shaders() const { return m_shaders; }
        const Container& programs() const { return m_programs; }
        const Container& textures2d() const { return m_textures2d; }

        void add_vertex_buffer(unsigned int id);
        void remove_vertex_buffer(unsigned int id);

        void add_element_buffer(unsigned int id);
        void remove_element_buffer(unsigned int id);

        void add_vertex_array(unsigned int id);
        void remove_vertex_array(unsigned int id);

        void add_shader(unsigned int id);
        void remove_shader(unsigned int id);

        void add_program(unsigned int id);
        void remove_program(unsigned int id);

        void add_texture2d(unsigned int id);
        void remove_texture2d(unsigned int id);
    private:
        Container m_vertex_buffers;
        Container m_element_buffers;
        Container m_vertex_arrays;
        Container m_shaders;
        Container m_programs;
        Container m_textures2d;
    };
}
