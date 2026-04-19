#include "hill/renderer.hpp"

#include <glad/gl.h>  // FIXME remove
#include <imgui.h>

namespace hill::renderer {
    Renderer::Renderer(const imgui::ImGui& imgui)
        : m_imgui(&imgui) {}

    Renderer::Renderer(const imgui::ImGui& imgui, const configuration::Configuration& configuration)
        : m_imgui(&imgui), m_configuration(configuration) {}

    Renderer::~Renderer() {

    }

    void Renderer::initialize() {
        imgui_initialize();

        glClearColor(0.4f, 0.1f, 0.6f, 1.0f);

        constexpr float vertices[] = {
            0.5f, -0.5f, 0.0f,
            0.0f, 0.5f, 0.0f,
            -0.5f, -0.5f, 0.0f
        };

        constexpr unsigned int indices[] = {
            0, 1, 2
        };

        const char* vertex_shader_source =
R"(
    #version 430 core

    layout(location = 0) in vec3 a_position;

    void main() {
        gl_Position = vec4(a_position, 1.0);
    }
)";

        const char* fragment_shader_source =
R"(
    #version 430 core

    out vec4 o_fragment;

    void main() {
        o_fragment = vec4(0.8, 0.2, 0.1, 1.0);
    }
)";

        glGenBuffers(1, &m_vertex_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glGenBuffers(1, &m_index_buffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index_buffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        glGenVertexArrays(1, &m_vertex_array);
        glBindVertexArray(m_vertex_array);
        glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), reinterpret_cast<void*>(0));
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index_buffer);
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        unsigned int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex_shader, 1, &vertex_shader_source, nullptr);
        glCompileShader(vertex_shader);

        unsigned int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment_shader, 1, &fragment_shader_source, nullptr);
        glCompileShader(fragment_shader);

        m_shader_program = glCreateProgram();
        glAttachShader(m_shader_program, vertex_shader);
        glAttachShader(m_shader_program, fragment_shader);
        glLinkProgram(m_shader_program);

        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
    }

    void Renderer::uninitialize() {
        glDeleteProgram(m_shader_program);
        glDeleteVertexArrays(1, &m_vertex_array);
        glDeleteBuffers(1, &m_index_buffer);
        glDeleteBuffers(1, &m_vertex_buffer);

        imgui_uninitialize();
    }

    void Renderer::render() {
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(m_shader_program);
        glBindVertexArray(m_vertex_array);
        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
        glUseProgram(0);

        imgui_render();
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

    void Renderer::imgui_render() {
        m_imgui->begin();
        ImGui::NewFrame();

        imgui();

        ImGui::EndFrame();
        ImGui::Render();
        m_imgui->end(ImGui::GetDrawData());
    }

    void Renderer::imgui() {
        ImGui::ShowDemoWindow();
    }
}
