#include "hill/renderer.hpp"

#include <ranges>
#include <cstring>

#include <imgui.h>
#include <ImGuizmo.h>

#include "hill/primitives/vertex_buffer.hpp"
#include "hill/primitives/element_buffer.hpp"
#include "hill/graphics_api.hpp"
#include "hill/renderer_command.hpp"
#include "hill/debug.hpp"
#include "hill/primitives_registry.hpp"
#include "hill/utility.hpp"

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
        primitives_registry::Registry::initialize();

        renderer_command::enable_depth_test();

        m_root_node = std::make_shared<scene::RootNode>();
        m_last_time = std::chrono::high_resolution_clock::now();
    }

    void Renderer::uninitialize() {
        m_root_node.reset();

        primitives_registry::Registry::uninitialize();
        imgui_uninitialize();
    }

    void Renderer::render() {
        const auto current_time = std::chrono::high_resolution_clock::now();
        m_frame_time = current_time - m_last_time;
        m_last_time = current_time;

        m_camera.projection(m_window_width, m_window_height, 45.0f, 0.01f, 100.0f);
        renderer_command::viewport(m_window_width, m_window_height);

        renderer_command::clear_color({ m_background_color[0], m_background_color[1], m_background_color[2], 1.0f });
        renderer_command::clear(renderer_command::Buffers::CD);

        m_camera.update_projection_view();

        for (const auto& weak_program : m_programs | std::views::values) {
            if (const auto program = weak_program.lock(); program) {
                program->use();
                program->upload_uniform_float16("u_projection_view", m_camera.projection_view());
                program->upload_uniform_float3("u_directional_light.direction", m_directional_light.direction);
                program->upload_uniform_float3("u_directional_light.ambient_color", m_directional_light.ambient_color);
                program->upload_uniform_float3("u_directional_light.diffuse_color", m_directional_light.diffuse_color);
                program->upload_uniform_float3("u_directional_light.specular_color", m_directional_light.specular_color);
                program->upload_uniform_float3("u_view_position", m_camera.position());
                program->unuse();
            }
        }

        TraversalCtx ctx;
        render_begin();
        render_traverse_tree(ctx, m_root_node.get());
        render_end();

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
        ImGuizmo::BeginFrame();

        m_imgui->update();

        ImGui::EndFrame();
        ImGui::Render();
        m_imgui->end(ImGui::GetDrawData());
    }

    void Renderer::submit(const RenderObject& object) {
        m_objects.emplace_back(object);
    }

    void Renderer::render_begin() {

    }

    void Renderer::render_end() {
        for (const RenderObject& object : m_objects) {
            draw_object(object);
        }

        m_objects.clear();
    }

    void Renderer::render_traverse_tree(TraversalCtx& ctx, scene::Node* node) {
        node->renderer_process(*this, ctx);

        for (const auto& child : node->m_children | std::views::values) {
            TraversalCtx new_ctx = ctx;
            render_traverse_tree(new_ctx, child.get());
        }
    }

    void Renderer::render_node(TraversalCtx& ctx, scene::RootNode* node) {

    }

    void Renderer::render_node(TraversalCtx& ctx, scene::ModelNode* node) {
        if (!node->m_runtime.configured) {
            configure(node);
            node->m_runtime.configured = true;
        }

        const auto translation = node->m_static.translation + node->translation;
        const auto rotation = node->m_static.rotation * glm::quat(glm::radians(node->rotation));
        const auto scale = node->m_static.scale * node->scale;

        glm::mat4 transform = glm::translate(glm::identity<glm::mat4>(), translation);
        transform *= glm::toMat4(rotation);
        transform = glm::scale(transform, scale);

        ctx.transform *= transform;

        for (const renderer_common::Object& object : node->m_runtime.objects) {
            submit(RenderObject { object, ctx.transform });
        }
    }

    void Renderer::render_node(TraversalCtx& ctx, scene::DirectionalLightNode* node) {
        m_directional_light = node->directional_light;
    }

    void Renderer::draw_object(const RenderObject& object) const {
        object.material->m_program->use();
        object.material->upload_data();
        object.vertex_array->bind();

        object.material->m_program->upload_uniform_float16("u_transform", object.transform);

        renderer_command::draw_elements_triangles(object.elements_count, object.elements_offset);

        object.vertex_array->unbind();
        object.material->m_program->unuse();
    }

    void Renderer::configure(scene::ModelNode* node) {
        node->m_runtime.objects.reserve(node->meshes_count());

        for (const auto& [i, mesh] : node->m_static.meshes | std::views::enumerate) {
            renderer_common::Object& object = node->m_runtime.objects.emplace_back();
            object.elements_count = int(mesh->indices.size());
            object.vertex_array = create_vertex_array(*mesh);
            object.material = node->meshes[std::size_t(i)].material;
            object.material->m_program = get_program(*mesh);
        }
    }

    std::shared_ptr<vertex_array::VertexArray> Renderer::create_vertex_array(const mesh::Mesh& mesh) const {
        const auto vertices_size = mesh.vertices.size() * (2 * sizeof(glm::vec3));  // TODO
        const auto vertices = std::make_unique<unsigned char[]>(vertices_size);

        for (std::size_t i {}; const mesh::Vertex& vertex : mesh.vertices) {
            std::memcpy(vertices.get() + i, glm::value_ptr(vertex.position), sizeof(vertex.position));
            i += sizeof(vertex.position);
            std::memcpy(vertices.get() + i, glm::value_ptr(vertex.normal), sizeof(vertex.normal));
            i += sizeof(vertex.normal);
        }

        const auto vertex_buffer = std::make_shared<vertex_buffer::VertexBuffer>();
        vertex_buffer->bind();
        vertex_buffer->upload_data(vertices.get(), vertices_size);
        vertex_buffer->unbind();

        const auto element_buffer = std::make_shared<element_buffer::ElementBuffer>();
        element_buffer->bind();
        element_buffer->upload_data(mesh.indices.data(), mesh.indices.size() * sizeof(unsigned int));
        element_buffer->unbind();

        vertex_array::Layout layout;
        layout.attribute(vertex_array::Attribute(0, 3, vertex_array::Type::Float, false, 2 * sizeof(glm::vec3), 0));  // TODO
        layout.attribute(vertex_array::Attribute(1, 3, vertex_array::Type::Float, false, 2 * sizeof(glm::vec3), sizeof(glm::vec3)));

        const auto vertex_array = std::make_shared<vertex_array::VertexArray>();
        vertex_array->bind();
        vertex_array->configure(vertex_buffer, layout);
        vertex_array->configure_and_unbind(element_buffer);

        return vertex_array;
    }

    std::shared_ptr<shader::Program> Renderer::create_program(renderer_common::ShaderFeatureSet shader_feature_set) {
        std::string vertex_shader_source;
        std::string fragment_shader_source;

        switch (shader_feature_set) {
            case renderer_common::ShaderFeatureBase:
                utility::read_file("shaders/basic.vert", vertex_shader_source);
                utility::read_file("shaders/basic.frag", fragment_shader_source);
                break;
        }

        const auto vertex_shader = std::make_shared<shader::Shader>(shader::ShaderType::Vertex);
        vertex_shader->compile(vertex_shader_source);

        const auto fragment_shader = std::make_shared<shader::Shader>(shader::ShaderType::Fragment);
        fragment_shader->compile(fragment_shader_source);

        const auto program = std::make_shared<shader::Program>();
        program->attach_shader(vertex_shader);
        program->attach_shader(fragment_shader);
        program->link();

        m_programs[shader_feature_set] = program;

        return program;
    }

    std::shared_ptr<shader::Program> Renderer::get_program(const mesh::Mesh& mesh) {
        const auto shader_feature_set = renderer_common::choose_shader_feature_set(mesh);

        if (const auto iter = m_programs.find(shader_feature_set); iter != m_programs.end()) {
            if (const auto program = iter->second.lock(); program) {
                return program;
            }
        }

        return create_program(shader_feature_set);
    }
}
