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
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<void*>(0));
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<void*>(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_index_buffer);
        glBindVertexArray(0);
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
        glClearColor(m_background_color[0], m_background_color[1], m_background_color[2], 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(m_shader_program);
        glBindVertexArray(m_vertex_array);
        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
        glUseProgram(0);

        glUseProgram(m_shader_program);
        glBindVertexArray(m_vertex_array);
        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, reinterpret_cast<void*>(3 * sizeof(unsigned int)));
        glBindVertexArray(0);
        glUseProgram(0);

        imgui_render();
    }

    void Renderer::window_resize(int width, int height) {
        glViewport(0, 0, width, height);
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

        ImGui::Begin("Test");
        ImGui::ColorEdit3("Background Color", m_background_color);
        ImGui::End();
    }
}
