#include "hill/renderer.hpp"

#include <ranges>
#include <cstring>

#include <imgui.h>

#include "hill/primitives/vertex_buffer.hpp"
#include "hill/primitives/element_buffer.hpp"
#include "hill/primitives/texture_2d.hpp"
#include "hill/graphics_api.hpp"
#include "hill/renderer_command.hpp"
#include "hill/debug.hpp"
#include "hill/primitives_registry.hpp"
#include "hill/file.hpp"

namespace hill::renderer {
    Renderer::Renderer(imgui::ImGui& imgui, const configuration::Configuration& configuration)
        : m_imgui(&imgui), m_configuration(configuration) {
        if (graphics_api::debug_context()) {
            debug::enable_graphics_api();
        }

        imgui_initialize();
        debug_initialize();
        skybox_initialize();
        render_initialize();

        m_root_node = std::make_shared<scene::RootNode>();
        m_performance.last_time = std::chrono::high_resolution_clock::now();

        renderer_command::enable_depth_test();
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

        m_camera.projection(m_window_width, m_window_height, 45.0f, 0.01f, 200.0f);
        renderer_command::viewport(m_window_width, m_window_height);

        renderer_command::clear_color({ m_background_color[0], m_background_color[1], m_background_color[2], 1.0f });
        renderer_command::clear(renderer_command::Buffers::CD);

        TraversalCtx ctx;
        render_begin();
        render_traverse_tree(ctx, m_root_node.get());
        render_end();

        skybox_render();
        debug_render();
        imgui_render();

        m_task_manager.update();
    }

    void Renderer::window_resize(int width, int height) {
        m_window_width = width;
        m_window_height = height;
    }

    void Renderer::skybox(environment::Skybox skybox) {
        m_environment.skybox = std::move(skybox);
        m_skybox_renderer.w_texture = m_environment.skybox.m_texture;
    }

    environment::Skybox Renderer::skybox(const environment::SkyboxFaces& skybox_faces) {
        texture_cubemap::Data data;
        data.structure.positive_x = skybox_faces.positive_x.data();
        data.structure.negative_x = skybox_faces.negative_x.data();
        data.structure.positive_y = skybox_faces.positive_y.data();
        data.structure.negative_y = skybox_faces.negative_y.data();
        data.structure.positive_z = skybox_faces.positive_z.data();
        data.structure.negative_z = skybox_faces.negative_z.data();

        auto texture = std::make_shared<texture_cubemap::TextureCubemap>(skybox_faces.positive_x.width(), skybox_faces.positive_x.height());
        texture->bind();
        texture->upload_data(data);
        texture->unbind();

        return environment::Skybox(std::move(texture));
    }

    void Renderer::fog(const environment::Fog& fog) {
        m_environment.fog = fog;

        static void (*set_dirty_flag)(scene::Node*) = [](scene::Node* node) {
            scene::ModelNode* model_node = dynamic_cast<scene::ModelNode*>(node);

            if (model_node) {
                model_node->m_program_dirty = true;
            }

            for (const auto& child : node->m_children | std::views::values) {
                set_dirty_flag(child.get());
            }
        };

        set_dirty_flag(m_root_node.get());
    }

    void Renderer::add_debug_line(glm::vec3 p1, glm::vec3 p2, glm::vec3 color) {
        m_debug_renderer.lines.emplace_back(p1, p2, color);
    }

    void Renderer::add_debug_aabb(const aabb::Aabb& aabb, glm::vec3 color) {
        const float size_x = aabb.max.x - aabb.min.x;
        const float size_y = aabb.max.y - aabb.min.y;
        const float size_z = aabb.max.z - aabb.min.z;

        m_debug_renderer.lines.emplace_back(aabb.min + glm::vec3(0.0f, 0.0f, 0.0f), aabb.min + glm::vec3(size_x, 0.0f, 0.0f), color);
        m_debug_renderer.lines.emplace_back(aabb.min + glm::vec3(0.0f, 0.0f, size_z), aabb.min + glm::vec3(size_x, 0.0f, size_z), color);
        m_debug_renderer.lines.emplace_back(aabb.min + glm::vec3(0.0f, 0.0f, 0.0f), aabb.min + glm::vec3(0.0f, 0.0f, size_z), color);
        m_debug_renderer.lines.emplace_back(aabb.min + glm::vec3(size_x, 0.0f, 0.0f), aabb.min + glm::vec3(size_x, 0.0f, size_z), color);

        m_debug_renderer.lines.emplace_back(aabb.min + glm::vec3(0.0f, 0.0f, 0.0f), aabb.min + glm::vec3(0.0f, size_y, 0.0f), color);
        m_debug_renderer.lines.emplace_back(aabb.min + glm::vec3(size_x, 0.0f, 0.0f), aabb.min + glm::vec3(size_x, size_y, 0.0f), color);
        m_debug_renderer.lines.emplace_back(aabb.min + glm::vec3(0.0f, 0.0f, size_z), aabb.min + glm::vec3(0.0f, size_y, size_z), color);
        m_debug_renderer.lines.emplace_back(aabb.min + glm::vec3(size_x, 0.0f, size_z), aabb.min + glm::vec3(size_x, size_y, size_z), color);

        m_debug_renderer.lines.emplace_back(aabb.min + glm::vec3(0.0f, size_y, 0.0f), aabb.min + glm::vec3(size_x, size_y, 0.0f), color);
        m_debug_renderer.lines.emplace_back(aabb.min + glm::vec3(0.0f, size_y, size_z), aabb.min + glm::vec3(size_x, size_y, size_z), color);
        m_debug_renderer.lines.emplace_back(aabb.min + glm::vec3(0.0f, size_y, 0.0f), aabb.min + glm::vec3(0.0f, size_y, size_z), color);
        m_debug_renderer.lines.emplace_back(aabb.min + glm::vec3(size_x, size_y, 0.0f), aabb.min + glm::vec3(size_x, size_y, size_z), color);
    }

    void Renderer::add_debug_aabb(const aabb::Aabb& aabb, const glm::mat4& transform, glm::vec3 color) {
        const float size_x = aabb.max.x - aabb.min.x;
        const float size_y = aabb.max.y - aabb.min.y;
        const float size_z = aabb.max.z - aabb.min.z;

        m_debug_renderer.lines.emplace_back(transform * glm::vec4(aabb.min + glm::vec3(0.0f, 0.0f, 0.0f), 1.0f), transform * glm::vec4(aabb.min + glm::vec3(size_x, 0.0f, 0.0f), 1.0f), color);
        m_debug_renderer.lines.emplace_back(transform * glm::vec4(aabb.min + glm::vec3(0.0f, 0.0f, size_z), 1.0f), transform * glm::vec4(aabb.min + glm::vec3(size_x, 0.0f, size_z), 1.0f), color);
        m_debug_renderer.lines.emplace_back(transform * glm::vec4(aabb.min + glm::vec3(0.0f, 0.0f, 0.0f), 1.0f), transform * glm::vec4(aabb.min + glm::vec3(0.0f, 0.0f, size_z), 1.0f), color);
        m_debug_renderer.lines.emplace_back(transform * glm::vec4(aabb.min + glm::vec3(size_x, 0.0f, 0.0f), 1.0f), transform * glm::vec4(aabb.min + glm::vec3(size_x, 0.0f, size_z), 1.0f), color);

        m_debug_renderer.lines.emplace_back(transform * glm::vec4(aabb.min + glm::vec3(0.0f, 0.0f, 0.0f), 1.0f), transform * glm::vec4(aabb.min + glm::vec3(0.0f, size_y, 0.0f), 1.0f), color);
        m_debug_renderer.lines.emplace_back(transform * glm::vec4(aabb.min + glm::vec3(size_x, 0.0f, 0.0f), 1.0f), transform * glm::vec4(aabb.min + glm::vec3(size_x, size_y, 0.0f), 1.0f), color);
        m_debug_renderer.lines.emplace_back(transform * glm::vec4(aabb.min + glm::vec3(0.0f, 0.0f, size_z), 1.0f), transform * glm::vec4(aabb.min + glm::vec3(0.0f, size_y, size_z), 1.0f), color);
        m_debug_renderer.lines.emplace_back(transform * glm::vec4(aabb.min + glm::vec3(size_x, 0.0f, size_z), 1.0f), transform * glm::vec4(aabb.min + glm::vec3(size_x, size_y, size_z), 1.0f), color);

        m_debug_renderer.lines.emplace_back(transform * glm::vec4(aabb.min + glm::vec3(0.0f, size_y, 0.0f), 1.0f), transform * glm::vec4(aabb.min + glm::vec3(size_x, size_y, 0.0f), 1.0f), color);
        m_debug_renderer.lines.emplace_back(transform * glm::vec4(aabb.min + glm::vec3(0.0f, size_y, size_z), 1.0f), transform * glm::vec4(aabb.min + glm::vec3(size_x, size_y, size_z), 1.0f), color);
        m_debug_renderer.lines.emplace_back(transform * glm::vec4(aabb.min + glm::vec3(0.0f, size_y, 0.0f), 1.0f), transform * glm::vec4(aabb.min + glm::vec3(0.0f, size_y, size_z), 1.0f), color);
        m_debug_renderer.lines.emplace_back(transform * glm::vec4(aabb.min + glm::vec3(size_x, size_y, 0.0f), 1.0f), transform * glm::vec4(aabb.min + glm::vec3(size_x, size_y, size_z), 1.0f), color);
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
            m_debug_renderer.w_vertex_buffer = vertex_buffer;

            vertex_array::LayoutAuto layout;
            layout.attribute(vertex_array::AttributeAuto(0, 3, vertex_array::Type::Float, false));
            layout.attribute(vertex_array::AttributeAuto(1, 3, vertex_array::Type::Float, false));

            m_debug_renderer.vertex_array = std::make_shared<vertex_array::VertexArray>();
            m_debug_renderer.vertex_array->bind();
            m_debug_renderer.vertex_array->configure(std::move(vertex_buffer), layout);
            m_debug_renderer.vertex_array->unbind();
        }

        {
            file::Buffer vertex_shader_source;
            file::Buffer fragment_shader_source;

            file::read("shaders/debug.vert", vertex_shader_source);
            file::read("shaders/debug.frag", fragment_shader_source);

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

        if (const auto vertex_buffer = m_debug_renderer.w_vertex_buffer.lock(); vertex_buffer) {
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

    void Renderer::skybox_initialize() {
        {
            auto vertex_buffer = std::make_shared<vertex_buffer::VertexBuffer>();
            vertex_buffer->bind();
            vertex_buffer->upload_data(environment::SKYBOX_POSITIONS, sizeof(environment::SKYBOX_POSITIONS));
            vertex_buffer->unbind();

            vertex_array::LayoutAuto layout;
            layout.attribute(vertex_array::AttributeAuto(0, 3, vertex_array::Type::Float, false));

            m_skybox_renderer.vertex_array = std::make_shared<vertex_array::VertexArray>();
            m_skybox_renderer.vertex_array->bind();
            m_skybox_renderer.vertex_array->configure(std::move(vertex_buffer), layout);
            m_skybox_renderer.vertex_array->unbind();
        }

        {
            file::Buffer vertex_shader_source;
            file::Buffer fragment_shader_source;

            file::read("shaders/skybox.vert", vertex_shader_source);
            file::read("shaders/skybox.frag", fragment_shader_source);

            auto vertex_shader = std::make_shared<shader::Shader>(shader::ShaderType::Vertex);
            vertex_shader->compile(vertex_shader_source);

            auto fragment_shader = std::make_shared<shader::Shader>(shader::ShaderType::Fragment);
            fragment_shader->compile(fragment_shader_source);

            m_skybox_renderer.program = std::make_shared<shader::Program>();
            m_skybox_renderer.program->attach_shader(std::move(vertex_shader));
            m_skybox_renderer.program->attach_shader(std::move(fragment_shader));
            m_skybox_renderer.program->link();
        }
    }

    void Renderer::skybox_render() const {
        const auto texture = m_skybox_renderer.w_texture.lock();

        if (!texture) {
            return;
        }

        m_skybox_renderer.program->use();
        m_skybox_renderer.vertex_array->bind();

        texture->bind();
        m_skybox_renderer.program->upload_uniform_float16("u_projection_view", m_camera.projection() * glm::mat4(glm::mat3(m_camera.view())));

        renderer_command::depth_function(renderer_command::DepthFunction::LEqual);

        renderer_command::draw_arrays_triangles(36);
        m_performance.draw_calls++;

        renderer_command::depth_function(renderer_command::DepthFunction::Less);

        m_skybox_renderer.vertex_array->unbind();
        m_skybox_renderer.program->unuse();
    }

    void Renderer::render_initialize() {
        file::read("shaders/phong.vert", m_phong_vertex_shader_source);
        file::read("shaders/phong.frag", m_phong_fragment_shader_source);
    }

    void Renderer::render_begin() {

    }

    void Renderer::render_end() {
        upload_program_shared_uniform_data();

        for (const RenderObject& object : m_main_renderer.objects) {
            draw_object(object);
        }

        m_main_renderer.objects.clear();
        m_main_renderer.directional_light = std::nullopt;
        m_main_renderer.point_lights.clear();
        m_main_renderer.spot_lights.clear();
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
            node->m_program_dirty = false;
            configure_render_objects(node);
        } else if (node->m_program_dirty) {
            node->m_program_dirty = false;
            configure_program(node);
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
        m_main_renderer.directional_light = node->directional_light;
    }

    void Renderer::render_node(TraversalCtx& ctx, scene::PointLightNode* node) {
        if (m_main_renderer.point_lights.size() >= 8) {
            return;
        }

        m_main_renderer.point_lights.push_back(node->point_light);
    }

    void Renderer::render_node(TraversalCtx& ctx, scene::SpotLightNode* node) {
        if (m_main_renderer.spot_lights.size() >= 8) {
            return;
        }

        m_main_renderer.spot_lights.push_back(node->spot_light);
    }

    void Renderer::submit(const RenderObject& object) {
        m_main_renderer.objects.emplace_back(object);
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

    void Renderer::configure_render_objects(scene::ModelNode* node) {
        node->m_render_objects.reserve(node->meshes_count());

        for (const auto& [raw_mesh, mesh] : std::views::zip(node->m_mesh_sources, node->m_meshes)) {
            renderer_common::Object& object = node->m_render_objects.emplace_back();
            object.elements_count = int(raw_mesh->indices.size());
            object.vertex_array = create_vertex_array(*raw_mesh);
            object.material = initialize_material(raw_mesh->material, mesh.material);
            object.material->m_program = get_or_create_program(renderer_common::choose_shader_feature_set(*raw_mesh, m_environment));
        }

        debug::callback()(debug::Type::Information, std::format("Configured model node {}", static_cast<void*>(node)));
    }

    void Renderer::configure_program(scene::ModelNode* node) {
        for (const auto& [raw_mesh, mesh, object] : std::views::zip(node->m_mesh_sources, node->m_meshes, node->m_render_objects)) {
            object.material->m_program = get_or_create_program(renderer_common::choose_shader_feature_set(*raw_mesh, m_environment));
        }

        debug::callback()(debug::Type::Information, std::format("Configured model node {}", static_cast<void*>(node)));
    }

    void Renderer::upload_program_shared_uniform_data() {
        for (const auto& [shader_feature_set, w_program] : m_main_renderer.programs) {
            if (const auto program = w_program.lock(); program) {
                program->use();

                program->upload_uniform_float16("u_projection", m_camera.projection());
                program->upload_uniform_float16("u_view", m_camera.view());

                if (m_main_renderer.directional_light) {
                    program->upload_uniform_float3("u_directional_light.color.ambient", m_main_renderer.directional_light->ambient_color);
                    program->upload_uniform_float3("u_directional_light.color.diffuse", m_main_renderer.directional_light->diffuse_color);
                    program->upload_uniform_float3("u_directional_light.color.specular", m_main_renderer.directional_light->specular_color);
                    program->upload_uniform_float3("u_directional_light.direction", m_main_renderer.directional_light->direction);
                } else {
                    program->upload_uniform_float3("u_directional_light.color.ambient", glm::vec3());
                    program->upload_uniform_float3("u_directional_light.color.diffuse", glm::vec3());
                    program->upload_uniform_float3("u_directional_light.color.specular", glm::vec3());
                    program->upload_uniform_float3("u_directional_light.direction", glm::vec3());
                }

                for (const auto& [i, point_light] : m_main_renderer.point_lights | std::views::enumerate) {
                    program->upload_uniform_float3(std::format("u_point_lights.elements[{}].color.ambient", i), point_light.ambient_color);
                    program->upload_uniform_float3(std::format("u_point_lights.elements[{}].color.diffuse", i), point_light.diffuse_color);
                    program->upload_uniform_float3(std::format("u_point_lights.elements[{}].color.specular", i), point_light.specular_color);
                    program->upload_uniform_float3(std::format("u_point_lights.elements[{}].position", i), point_light.position);
                    program->upload_uniform_float1(std::format("u_point_lights.elements[{}].linear", i), point_light.linear);
                    program->upload_uniform_float1(std::format("u_point_lights.elements[{}].quadratic", i), point_light.quadratic);
                }
                program->upload_uniform_uint1("u_point_lights.count", static_cast<unsigned int>(m_main_renderer.point_lights.size()));

                for (const auto& [i, spot_light] : m_main_renderer.spot_lights | std::views::enumerate) {
                    program->upload_uniform_float3(std::format("u_spot_lights.elements[{}].color.ambient", i), spot_light.ambient_color);
                    program->upload_uniform_float3(std::format("u_spot_lights.elements[{}].color.diffuse", i), spot_light.diffuse_color);
                    program->upload_uniform_float3(std::format("u_spot_lights.elements[{}].color.specular", i), spot_light.specular_color);
                    program->upload_uniform_float3(std::format("u_spot_lights.elements[{}].position", i), spot_light.position);
                    program->upload_uniform_float3(std::format("u_spot_lights.elements[{}].direction", i), spot_light.direction);
                    program->upload_uniform_float1(std::format("u_spot_lights.elements[{}].linear", i), spot_light.linear);
                    program->upload_uniform_float1(std::format("u_spot_lights.elements[{}].quadratic", i), spot_light.quadratic);
                    program->upload_uniform_float1(std::format("u_spot_lights.elements[{}].cutoff_inner", i), glm::cos(glm::radians(spot_light.cutoff_inner)));
                    program->upload_uniform_float1(std::format("u_spot_lights.elements[{}].cutoff_outer", i), glm::cos(glm::radians(spot_light.cutoff_outer)));
                }
                program->upload_uniform_uint1("u_spot_lights.count", static_cast<unsigned int>(m_main_renderer.spot_lights.size()));

                program->upload_uniform_float3("u_view_position", m_camera.position());

                if (shader_feature_set & renderer_common::ShaderFeatureFog) {
                    program->upload_uniform_float1("u_fog_density", m_environment.fog.density);
                    program->upload_uniform_float1("u_fog_gradient", m_environment.fog.gradient);
                    program->upload_uniform_float3("u_fog_color", m_environment.fog.color);
                }

                program->unuse();
            }
        }
    }

    std::shared_ptr<vertex_array::VertexArray> Renderer::create_vertex_array(const mesh::MeshSource& mesh_source) const {
        const auto vertices_size = [&] {
            auto size = 2 * sizeof(glm::vec3);

            if (mesh_source.vertex_attributes & mesh::TextureCoordinates) {
                size += sizeof(glm::vec2);
            }

            return size * mesh_source.vertices.size();
        }();

        const auto vertices = std::make_unique<unsigned char[]>(vertices_size);

        for (std::size_t i {}; const mesh::Vertex& vertex : mesh_source.vertices) {
            std::memcpy(vertices.get() + i, glm::value_ptr(vertex.position), sizeof(vertex.position));
            i += sizeof(vertex.position);
            std::memcpy(vertices.get() + i, glm::value_ptr(vertex.normal), sizeof(vertex.normal));
            i += sizeof(vertex.normal);

            if (mesh_source.vertex_attributes & mesh::TextureCoordinates) {
                std::memcpy(vertices.get() + i, glm::value_ptr(vertex.texture_coordinate), sizeof(vertex.texture_coordinate));
                i += sizeof(vertex.texture_coordinate);
            }
        }

        auto vertex_buffer = std::make_shared<vertex_buffer::VertexBuffer>();
        vertex_buffer->bind();
        vertex_buffer->upload_data(vertices.get(), vertices_size);
        vertex_buffer->unbind();

        auto element_buffer = std::make_shared<element_buffer::ElementBuffer>();
        element_buffer->bind();
        element_buffer->upload_data(mesh_source.indices.data(), mesh_source.indices.size() * sizeof(unsigned int));
        element_buffer->unbind();

        vertex_array::LayoutAuto layout;
        layout.attribute(vertex_array::AttributeAuto(0, 3, vertex_array::Type::Float, false));
        layout.attribute(vertex_array::AttributeAuto(1, 3, vertex_array::Type::Float, false));

        if (mesh_source.vertex_attributes & mesh::TextureCoordinates) {
            layout.attribute(vertex_array::AttributeAuto(3, 2, vertex_array::Type::Float, false));
        }

        const auto vertex_array = std::make_shared<vertex_array::VertexArray>();
        vertex_array->bind();
        vertex_array->configure(std::move(vertex_buffer), layout);
        vertex_array->configure_and_unbind(std::move(element_buffer));

        return vertex_array;
    }

    std::shared_ptr<shader::Program> Renderer::create_program(renderer_common::ShaderFeatureSet shader_feature_set) const {
        auto vertex_shader = std::make_shared<shader::Shader>(shader::ShaderType::Vertex);
        vertex_shader->compile(create_vertex_shader_sources(shader_feature_set));

        auto fragment_shader = std::make_shared<shader::Shader>(shader::ShaderType::Fragment);
        fragment_shader->compile(create_fragment_shader_sources(shader_feature_set));

        const auto program = std::make_shared<shader::Program>();
        program->attach_shader(std::move(vertex_shader));
        program->attach_shader(std::move(fragment_shader));
        program->link();

        return program;
    }

    std::shared_ptr<shader::Program> Renderer::get_or_create_program(renderer_common::ShaderFeatureSet shader_feature_set) {
        if (const auto iter = m_main_renderer.programs.find(shader_feature_set); iter != m_main_renderer.programs.end()) {
            if (const auto program = iter->second.lock(); program) {
                return program;
            }
        }

        const auto program = create_program(shader_feature_set);
        m_main_renderer.programs[shader_feature_set] = program;

        return program;
    }

    std::shared_ptr<material::Material> Renderer::initialize_material(const mesh::Material& raw_material, std::shared_ptr<material::Material> material) {
        if (const auto iter = material->m_floats3.find("u_material.color_ambient"); iter != material->m_floats3.end()) {
            iter->second = raw_material.color_ambient;
        }

        if (const auto iter = material->m_floats3.find("u_material.color_diffuse"); iter != material->m_floats3.end()) {
            iter->second = raw_material.color_diffuse;
        }

        if (const auto iter = material->m_floats3.find("u_material.color_specular"); iter != material->m_floats3.end()) {
            iter->second = raw_material.color_specular;
        }

        if (const auto iter = material->m_floats1.find("u_material.shininess"); iter != material->m_floats1.end()) {
            iter->second = raw_material.shininess;
        }

        if (const auto iter = material->m_textures.find("u_material.texture_diffuse"); iter != material->m_textures.end()) {
            iter->second = std::make_pair(get_or_create_texture(raw_material.texture_diffuse), 0);
        }

        if (const auto iter = material->m_textures.find("u_material.texture_specular"); iter != material->m_textures.end()) {
            iter->second = std::make_pair(get_or_create_texture(raw_material.texture_specular), 1);
        }

        return material;
    }

    std::shared_ptr<texture_2d::Texture2D> Renderer::get_or_create_texture(mesh::TextureSource texture_source) {
        if (const auto iter = m_main_renderer.textures.find(texture_source); iter != m_main_renderer.textures.end()) {
            if (const auto texture = iter->second.lock(); texture) {
                return texture;
            }
        }

        const auto texture = std::make_shared<texture_2d::Texture2D>(texture_2d::Format::Rgba8, texture_source->width(), texture_source->height());
        texture->bind();
        texture->upload_data(texture_source->data());
        texture->unbind();

        m_main_renderer.textures[std::move(texture_source)] = texture;

        debug::callback()(debug::Type::Debug, std::format("Created texture {}", texture->id()));

        return texture;
    }

    std::vector<std::string> Renderer::create_vertex_shader_sources(renderer_common::ShaderFeatureSet shader_feature_set) const {
        std::vector<std::string> sources;

        sources.push_back("#version 430 core\n\n");
        setup_shader_features(shader_feature_set, sources);
        sources.push_back(m_phong_vertex_shader_source);

        return sources;
    }

    std::vector<std::string> Renderer::create_fragment_shader_sources(renderer_common::ShaderFeatureSet shader_feature_set) const {
        std::vector<std::string> sources;

        sources.push_back("#version 430 core\n\n");
        setup_shader_features(shader_feature_set, sources);
        sources.push_back(m_phong_fragment_shader_source);

        return sources;
    }

    void Renderer::setup_shader_features(renderer_common::ShaderFeatureSet shader_feature_set, std::vector<std::string>& sources) {
        const auto define = [](const char* name) {
            return std::format("#define {}\n", name);
        };

        bool texture_coordinates {};

        if (shader_feature_set & renderer_common::ShaderFeatureVertexColors) {
            sources.push_back(define("FEATURE_VERTEX_COLORS"));
        }

        if (shader_feature_set & renderer_common::ShaderFeatureDiffuseMap) {
            sources.push_back(define("FEATURE_DIFFUSE_MAP"));
            texture_coordinates = true;
        }

        if (shader_feature_set & renderer_common::ShaderFeatureSpecularMap) {
            sources.push_back(define("FEATURE_SPECULAR_MAP"));
            texture_coordinates = true;
        }

        if (shader_feature_set & renderer_common::ShaderFeatureNormalMap) {
            sources.push_back(define("FEATURE_NORMAL_MAP"));
            texture_coordinates = true;
        }

        if (shader_feature_set & renderer_common::ShaderFeatureEmissionMap) {
            sources.push_back(define("FEATURE_EMISSION_MAP"));
            texture_coordinates = true;
        }

        if (shader_feature_set & renderer_common::ShaderFeatureFog) {
            sources.push_back(define("FEATURE_FOG"));
        }

        if (shader_feature_set & renderer_common::ShaderFeatureShadow) {
            sources.push_back(define("FEATURE_SHADOW"));
        }

        if (texture_coordinates) {
            sources.push_back(define("META_FEATURE_TEXTURE_COORDINATES"));
        }
    }
}
