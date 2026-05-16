#include "hill/renderer.hpp"

#include <ranges>
#include <cstring>

#include <imgui.h>

#include "hill/primitives/vertex_buffer.hpp"
#include "hill/primitives/element_buffer.hpp"
#include "hill/graphics_api.hpp"
#include "hill/renderer_command.hpp"
#include "hill/debug.hpp"
#include "hill/primitives_registry.hpp"
#include "hill/utility.hpp"

namespace hill::renderer {
    Renderer::Renderer(imgui::ImGui& imgui, const configuration::Configuration& configuration)
        : m_imgui(&imgui), m_configuration(configuration) {
        if (m_configuration.debug_output && graphics_api::debug_context()) {
            debug::initialize(*m_configuration.debug_output);
        }

        debug_initialize();
        imgui_initialize();

        renderer_command::enable_depth_test();

        m_root_node = std::make_shared<scene::RootNode>();
        m_performance.last_time = std::chrono::high_resolution_clock::now();
    }

    Renderer::~Renderer() {
        imgui_uninitialize();
        primitives_registry::uninitialize();
    }

    void Renderer::render() {
        {
            const auto current_time = std::chrono::high_resolution_clock::now();
            m_performance.frame_time = current_time - m_performance.last_time;
            m_performance.last_time = current_time;

            m_performance.draw_calls = 0;
            m_performance.transform_updates = 0;
        }

        m_camera.projection(m_window_width, m_window_height, 45.0f, 0.01f, 100.0f);
        renderer_command::viewport(m_window_width, m_window_height);

        renderer_command::clear_color({ m_background_color[0], m_background_color[1], m_background_color[2], 1.0f });
        renderer_command::clear(renderer_command::Buffers::CD);

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

        debug_render();
        imgui_render();
    }

    void Renderer::window_resize(int width, int height) {
        m_window_width = width;
        m_window_height = height;
    }

    void Renderer::add_debug_line(glm::vec3 p1, glm::vec3 p2, glm::vec3 color) {
        m_debug_renderer.lines.emplace_back(p1, p2, color);
    }

    void Renderer::add_debug_aabb(const aabb::Aabb& aabb, glm::vec3 color) {
        const float size_x = aabb.max.x - aabb.min.x;
        const float size_y = aabb.max.y - aabb.min.y;
        const float size_z = aabb.max.z - aabb.min.z;

        m_debug_renderer.lines.emplace_back(glm::vec3(aabb.min) + glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(aabb.min) + glm::vec3(size_x, 0.0f, 0.0f), color);
        m_debug_renderer.lines.emplace_back(glm::vec3(aabb.min) + glm::vec3(0.0f, 0.0f, size_z), glm::vec3(aabb.min) + glm::vec3(size_x, 0.0f, size_z), color);
        m_debug_renderer.lines.emplace_back(glm::vec3(aabb.min) + glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(aabb.min) + glm::vec3(0.0f, 0.0f, size_z), color);
        m_debug_renderer.lines.emplace_back(glm::vec3(aabb.min) + glm::vec3(size_x, 0.0f, 0.0f), glm::vec3(aabb.min) + glm::vec3(size_x, 0.0f, size_z), color);

        m_debug_renderer.lines.emplace_back(glm::vec3(aabb.min) + glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(aabb.min) + glm::vec3(0.0f, size_y, 0.0f), color);
        m_debug_renderer.lines.emplace_back(glm::vec3(aabb.min) + glm::vec3(size_x, 0.0f, 0.0f), glm::vec3(aabb.min) + glm::vec3(size_x, size_y, 0.0f), color);
        m_debug_renderer.lines.emplace_back(glm::vec3(aabb.min) + glm::vec3(0.0f, 0.0f, size_z), glm::vec3(aabb.min) + glm::vec3(0.0f, size_y, size_z), color);
        m_debug_renderer.lines.emplace_back(glm::vec3(aabb.min) + glm::vec3(size_x, 0.0f, size_z), glm::vec3(aabb.min) + glm::vec3(size_x, size_y, size_z), color);

        m_debug_renderer.lines.emplace_back(glm::vec3(aabb.min) + glm::vec3(0.0f, size_y, 0.0f), glm::vec3(aabb.min) + glm::vec3(size_x, size_y, 0.0f), color);
        m_debug_renderer.lines.emplace_back(glm::vec3(aabb.min) + glm::vec3(0.0f, size_y, size_z), glm::vec3(aabb.min) + glm::vec3(size_x, size_y, size_z), color);
        m_debug_renderer.lines.emplace_back(glm::vec3(aabb.min) + glm::vec3(0.0f, size_y, 0.0f), glm::vec3(aabb.min) + glm::vec3(0.0f, size_y, size_z), color);
        m_debug_renderer.lines.emplace_back(glm::vec3(aabb.min) + glm::vec3(size_x, size_y, 0.0f), glm::vec3(aabb.min) + glm::vec3(size_x, size_y, size_z), color);
    }

    void Renderer::add_debug_aabb(const aabb::Aabb& aabb, const glm::mat4& transform, glm::vec3 color) {
        const float size_x = aabb.max.x - aabb.min.x;
        const float size_y = aabb.max.y - aabb.min.y;
        const float size_z = aabb.max.z - aabb.min.z;

        m_debug_renderer.lines.emplace_back(transform * glm::vec4(glm::vec3(aabb.min) + glm::vec3(0.0f, 0.0f, 0.0f), 1.0f), transform * glm::vec4(glm::vec3(aabb.min) + glm::vec3(size_x, 0.0f, 0.0f), 1.0f), color);
        m_debug_renderer.lines.emplace_back(transform * glm::vec4(glm::vec3(aabb.min) + glm::vec3(0.0f, 0.0f, size_z), 1.0f), transform * glm::vec4(glm::vec3(aabb.min) + glm::vec3(size_x, 0.0f, size_z), 1.0f), color);
        m_debug_renderer.lines.emplace_back(transform * glm::vec4(glm::vec3(aabb.min) + glm::vec3(0.0f, 0.0f, 0.0f), 1.0f), transform * glm::vec4(glm::vec3(aabb.min) + glm::vec3(0.0f, 0.0f, size_z), 1.0f), color);
        m_debug_renderer.lines.emplace_back(transform * glm::vec4(glm::vec3(aabb.min) + glm::vec3(size_x, 0.0f, 0.0f), 1.0f), transform * glm::vec4(glm::vec3(aabb.min) + glm::vec3(size_x, 0.0f, size_z), 1.0f), color);

        m_debug_renderer.lines.emplace_back(transform * glm::vec4(glm::vec3(aabb.min) + glm::vec3(0.0f, 0.0f, 0.0f), 1.0f), transform * glm::vec4(glm::vec3(aabb.min) + glm::vec3(0.0f, size_y, 0.0f), 1.0f), color);
        m_debug_renderer.lines.emplace_back(transform * glm::vec4(glm::vec3(aabb.min) + glm::vec3(size_x, 0.0f, 0.0f), 1.0f), transform * glm::vec4(glm::vec3(aabb.min) + glm::vec3(size_x, size_y, 0.0f), 1.0f), color);
        m_debug_renderer.lines.emplace_back(transform * glm::vec4(glm::vec3(aabb.min) + glm::vec3(0.0f, 0.0f, size_z), 1.0f), transform * glm::vec4(glm::vec3(aabb.min) + glm::vec3(0.0f, size_y, size_z), 1.0f), color);
        m_debug_renderer.lines.emplace_back(transform * glm::vec4(glm::vec3(aabb.min) + glm::vec3(size_x, 0.0f, size_z), 1.0f), transform * glm::vec4(glm::vec3(aabb.min) + glm::vec3(size_x, size_y, size_z), 1.0f), color);

        m_debug_renderer.lines.emplace_back(transform * glm::vec4(glm::vec3(aabb.min) + glm::vec3(0.0f, size_y, 0.0f), 1.0f), transform * glm::vec4(glm::vec3(aabb.min) + glm::vec3(size_x, size_y, 0.0f), 1.0f), color);
        m_debug_renderer.lines.emplace_back(transform * glm::vec4(glm::vec3(aabb.min) + glm::vec3(0.0f, size_y, size_z), 1.0f), transform * glm::vec4(glm::vec3(aabb.min) + glm::vec3(size_x, size_y, size_z), 1.0f), color);
        m_debug_renderer.lines.emplace_back(transform * glm::vec4(glm::vec3(aabb.min) + glm::vec3(0.0f, size_y, 0.0f), 1.0f), transform * glm::vec4(glm::vec3(aabb.min) + glm::vec3(0.0f, size_y, size_z), 1.0f), color);
        m_debug_renderer.lines.emplace_back(transform * glm::vec4(glm::vec3(aabb.min) + glm::vec3(size_x, size_y, 0.0f), 1.0f), transform * glm::vec4(glm::vec3(aabb.min) + glm::vec3(size_x, size_y, size_z), 1.0f), color);
    }

    void Renderer::imgui_initialize() const {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        m_imgui->imgui_initialize();
    }

    void Renderer::imgui_uninitialize() const {
        m_imgui->imgui_uninitialize();
        ImGui::DestroyContext();
    }

    void Renderer::imgui_render() const {
        m_imgui->imgui_begin();
        ImGui::NewFrame();

        m_imgui->imgui_update();

        ImGui::EndFrame();
        ImGui::Render();
        m_imgui->imgui_end(ImGui::GetDrawData());
    }

    void Renderer::debug_initialize() {
        {
            auto vertex_buffer = std::make_shared<vertex_buffer::VertexBuffer>();
            m_debug_renderer.weak_vertex_buffer = vertex_buffer;

            vertex_array::Layout layout;
            layout.attribute(vertex_array::Attribute(0, 3, vertex_array::Type::Float, false, (3 + 3) * sizeof(float), 0));
            layout.attribute(vertex_array::Attribute(1, 3, vertex_array::Type::Float, false, (3 + 3) * sizeof(float), 3 * sizeof(float)));

            m_debug_renderer.vertex_array = std::make_shared<vertex_array::VertexArray>();
            m_debug_renderer.vertex_array->bind();
            m_debug_renderer.vertex_array->configure(std::move(vertex_buffer), layout);
            m_debug_renderer.vertex_array->unbind();
        }

        {
            std::string vertex_shader_source;
            std::string fragment_shader_source;

            utility::read_file("shaders/debug.vert", vertex_shader_source);
            utility::read_file("shaders/debug.frag", fragment_shader_source);

            auto vertex_shader = std::make_shared<shader::Shader>(shader::ShaderType::Vertex);
            vertex_shader->compile(vertex_shader_source);

            auto fragment_shader = std::make_shared<shader::Shader>(shader::ShaderType::Fragment);
            fragment_shader->compile(fragment_shader_source);

            m_debug_renderer.program = std::make_shared<shader::Program>();
            m_debug_renderer.program->attach_shader(std::move(vertex_shader));
            m_debug_renderer.program->attach_shader(std::move(fragment_shader));
            m_debug_renderer.program->link();
        }
    }

    void Renderer::debug_render() {
        if (m_debug_renderer.lines.empty()) {
            return;
        }

        struct Vertex {
            glm::vec3 position {};
            glm::vec3 color {};
        };

        std::vector<Vertex> vertices;

        for (const DebugRenderer::Line& line : m_debug_renderer.lines) {
            vertices.push_back({ line.p1, line.color });
            vertices.push_back({ line.p2, line.color });
        }

        if (const auto vertex_buffer = m_debug_renderer.weak_vertex_buffer.lock(); vertex_buffer) {
            vertex_buffer->bind();
            vertex_buffer->upload_data(vertices.data(), vertices.size() * sizeof(decltype(vertices)::value_type), common::BufferUsage::StreamDraw);
            vertex_buffer->unbind();
        }

        m_debug_renderer.program->use();
        m_debug_renderer.vertex_array->bind();

        m_debug_renderer.program->upload_uniform_float16("u_projection_view", m_camera.projection_view());

        renderer_command::draw_arrays_lines(int(vertices.size()));
        m_performance.draw_calls++;

        m_debug_renderer.vertex_array->unbind();
        m_debug_renderer.program->unuse();

        m_debug_renderer.lines.clear();
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
        if (node->m_render_objects_dirty) {
            node->m_render_objects_dirty = false;
            configure(node);
        }

        const auto local_transform =
            glm::translate(glm::identity<glm::mat4>(), node->m_local.translation) *
            glm::mat4_cast(node->m_local.rotation) *
            glm::scale(glm::identity<glm::mat4>(), node->m_local.scale);

        ctx.dirty |= node->m_world_transform_dirty;

        if (ctx.dirty) {
            node->m_world_transform_dirty = false;
            node->m_world_transform = ctx.parent_world_transform * local_transform;
            m_performance.transform_updates++;
        }

        ctx.parent_world_transform = node->m_world_transform;

        for (const renderer_common::Object& object : node->m_render_objects) {
            submit(RenderObject { object, node->m_world_transform });
        }
    }

    void Renderer::render_node(TraversalCtx& ctx, scene::DirectionalLightNode* node) {
        m_directional_light = node->directional_light;
    }

    void Renderer::draw_object(const RenderObject& object) const {
        object.material->m_program->use();
        object.material->upload_data();
        object.vertex_array->bind();

        object.material->m_program->upload_uniform_float16("u_transform", object.world_transform);

        renderer_command::draw_elements_triangles(object.elements_count, object.elements_offset);
        m_performance.draw_calls++;

        object.vertex_array->unbind();
        object.material->m_program->unuse();
    }

    void Renderer::configure(scene::ModelNode* node) {
        node->m_render_objects.reserve(node->meshes_count());

        for (const auto& [i, mesh] : node->m_raw_meshes | std::views::enumerate) {
            renderer_common::Object& object = node->m_render_objects.emplace_back();
            object.elements_count = int(mesh->indices.size());
            object.vertex_array = create_vertex_array(*mesh);
            object.material = node->m_meshes[std::size_t(i)].material;
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

        auto vertex_buffer = std::make_shared<vertex_buffer::VertexBuffer>();
        vertex_buffer->bind();
        vertex_buffer->upload_data(vertices.get(), vertices_size);
        vertex_buffer->unbind();

        auto element_buffer = std::make_shared<element_buffer::ElementBuffer>();
        element_buffer->bind();
        element_buffer->upload_data(mesh.indices.data(), mesh.indices.size() * sizeof(unsigned int));
        element_buffer->unbind();

        vertex_array::Layout layout;
        layout.attribute(vertex_array::Attribute(0, 3, vertex_array::Type::Float, false, 2 * sizeof(glm::vec3), 0));  // TODO
        layout.attribute(vertex_array::Attribute(1, 3, vertex_array::Type::Float, false, 2 * sizeof(glm::vec3), sizeof(glm::vec3)));

        const auto vertex_array = std::make_shared<vertex_array::VertexArray>();
        vertex_array->bind();
        vertex_array->configure(std::move(vertex_buffer), layout);
        vertex_array->configure_and_unbind(std::move(element_buffer));

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

        auto vertex_shader = std::make_shared<shader::Shader>(shader::ShaderType::Vertex);
        vertex_shader->compile(vertex_shader_source);

        auto fragment_shader = std::make_shared<shader::Shader>(shader::ShaderType::Fragment);
        fragment_shader->compile(fragment_shader_source);

        const auto program = std::make_shared<shader::Program>();
        program->attach_shader(std::move(vertex_shader));
        program->attach_shader(std::move(fragment_shader));
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
