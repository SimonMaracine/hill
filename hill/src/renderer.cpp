#include "hill/renderer.hpp"

#include <imgui.h>

#include "hill/graphics_api.hpp"
#include "hill/renderer_command.hpp"
#include "hill/debug.hpp"

namespace hill::renderer {
    Renderer::Renderer(imgui::ImGui& imgui)
        : m_imgui(&imgui) {}

    Renderer::Renderer(imgui::ImGui& imgui, const configuration::Configuration& configuration)
        : m_imgui(&imgui), m_configuration(configuration) {}

    void Renderer::initialize() {
        if (m_configuration.debug_output && graphics_api::debug_context()) {
            debug::initialize(*m_configuration.debug_output);
        }

        imgui_initialize();

        constexpr float vertices[] = {
            0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.75f, 0.0f, 1.0f, 0.0f, 0.0f,
            -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
            0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
            -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, -0.75f, 0.0f, 0.0f, 0.0f, 1.0f
        };

        constexpr unsigned int indices[] = {
            0, 1, 2, 3, 4, 5
        };

        const char* vertex_shader_source =
R"(
    #version 430 core

    layout(location = 0) in vec3 a_position;
    layout(location = 1) in vec3 a_color;

    out vec3 v_color;

    void main() {
        gl_Position = vec4(a_position, 1.0);
        v_color = a_color;
    }
)";

        const char* fragment_shader_source =
R"(
    #version 430 core

    in vec3 v_color;

    layout(location = 0) out vec4 o_fragment;

    void main() {
        o_fragment = vec4(v_color, 1.0);
    }
)";

        m_vertex_buffer = std::make_shared<vertex_buffer::VertexBuffer>();
        m_vertex_buffer->bind();
        m_vertex_buffer->upload_data(vertices, sizeof(vertices));
        m_vertex_buffer->unbind();

        m_element_buffer = std::make_shared<element_buffer::ElementBuffer>();
        m_element_buffer->bind();
        m_element_buffer->upload_data(indices, sizeof(indices));
        m_element_buffer->unbind();

        vertex_array::Layout layout;
        layout.attribute(vertex_array::Attribute(0, 3, vertex_array::Type::Float, false, 6 * sizeof(float), 0));
        layout.attribute(vertex_array::Attribute(1, 3, vertex_array::Type::Float, false, 6 * sizeof(float), 3 * sizeof(float)));

        m_vertex_array = std::make_shared<vertex_array::VertexArray>();
        m_vertex_array->bind();
        m_vertex_array->configure(m_vertex_buffer, layout);
        m_vertex_array->configure_and_unbind(m_element_buffer);

        const auto vertex_shader = std::make_shared<shader::Shader>(shader::ShaderType::Vertex);
        vertex_shader->compile(vertex_shader_source);

        const auto fragment_shader = std::make_shared<shader::Shader>(shader::ShaderType::Fragment);
        fragment_shader->compile(fragment_shader_source);

        m_program = std::make_shared<shader::Program>();
        m_program->attach_shader(vertex_shader);
        m_program->attach_shader(fragment_shader);
        m_program->link();
    }

    void Renderer::uninitialize() {
        m_program.reset();
        m_vertex_array.reset();
        m_element_buffer.reset();
        m_vertex_buffer.reset();

        imgui_uninitialize();
    }

    void Renderer::render() {
        renderer_command::viewport(m_window_width, m_window_height);

        renderer_command::clear_color(m_background_color[0], m_background_color[1], m_background_color[2], 1.0f);
        renderer_command::clear(renderer_command::Buffers::C);

        submit(m_program, m_vertex_array, 3, 0);
        submit(m_program, m_vertex_array, 3, 3);

        imgui_render();
    }

    void Renderer::window_resize(int width, int height) {
        m_window_width = width;
        m_window_height = height;
    }

    void Renderer::imgui_initialize() const {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        m_imgui->initialize();
    }

    void Renderer::imgui_uninitialize() const {
        m_imgui->uninitialize();
        ImGui::DestroyContext();
    }

    void Renderer::imgui_render() const {
        m_imgui->begin();
        ImGui::NewFrame();

        m_imgui->update();

        ImGui::EndFrame();
        ImGui::Render();
        m_imgui->end(ImGui::GetDrawData());
    }

    void Renderer::submit(std::shared_ptr<shader::Program> program, std::shared_ptr<vertex_array::VertexArray> vertex_array, int count, int offset) {
        program->use();
        vertex_array->bind();
        renderer_command::draw_elements_triangles(count, offset);
        vertex_array->unbind();
        program->unuse();
    }
}
