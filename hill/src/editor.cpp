#include "hill/editor.hpp"

#include <ranges>
#include <limits>
#include <cstring>

#include <imgui.h>

#include "hill/renderer.hpp"
#include "hill/primitives_registry.hpp"
#include "hill/scene.hpp"

namespace hill::editor {
    static bool input_text(const char* label, char* buffer, std::size_t size) {
        if (ImGui::InputText(label, buffer, size, ImGuiInputTextFlags_EnterReturnsTrue)) {
            if (*buffer != 0) {
                return true;
            }
        }

        return false;
    }

    static void separator() {
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
    }

    void ModelMesh::editor_inspect(Editor& editor) {
        editor.inspect(this);
    }

    Editor::Editor(windowing::Windowing& windowing)
        : m_windowing(&windowing) {
        auto& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    }

    void Editor::update(renderer::Renderer& renderer) {
        ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

        ImGui::ShowDemoWindow();

        performance(renderer);
        Editor::renderer(renderer);
        primitives_registry(renderer);
        scene_hierarchy(renderer);
        inspector(renderer);
        world_origin(renderer);
        world_grid(renderer);
        world_bounding_box(renderer);
    }

    void Editor::update_camera(renderer::Renderer& renderer) {
        static constexpr float MOVE_SPEED = 10.0f;
        static constexpr float LOOK_SPEED = 7.0f;

        if (!ImGui::GetIO().WantCaptureMouse) {
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
                m_windowing->windowing_grab_mouse();
                m_camera.control = true;
            }

            if (ImGui::IsMouseReleased(ImGuiMouseButton_Right)) {
                m_windowing->windowing_ungrab_mouse();
                m_camera.control = false;
            }

            if (m_camera.control && ImGui::IsMouseDragging(ImGuiMouseButton_Right)) {
                const ImVec2 delta = ImGui::GetIO().MouseDelta;
                m_camera.yaw += delta.x * LOOK_SPEED * ImGui::GetIO().DeltaTime;
                m_camera.pitch += -delta.y * LOOK_SPEED * ImGui::GetIO().DeltaTime;
            }
        }

        m_camera.pitch = glm::min(m_camera.pitch, 89.0f);
        m_camera.pitch = glm::max(m_camera.pitch, -89.0f);

        if (!ImGui::GetIO().WantCaptureKeyboard) {
            if (m_camera.control && ImGui::IsKeyDown(ImGuiKey_LeftShift)) {
                m_camera.move_speed_multiplier = 5.0f;
            } else {
                m_camera.move_speed_multiplier = 1.0f;
            }

            glm::vec3 position_offset {};

            if (m_camera.control && ImGui::IsKeyDown(ImGuiKey_W)) {
                position_offset += m_camera.direction;
            }

            if (m_camera.control && ImGui::IsKeyDown(ImGuiKey_S)) {
                position_offset -= m_camera.direction;
            }

            if (m_camera.control && ImGui::IsKeyDown(ImGuiKey_A)) {
                position_offset -= glm::normalize(glm::cross(m_camera.direction, m_camera.UP));
            }

            if (m_camera.control && ImGui::IsKeyDown(ImGuiKey_D)) {
                position_offset += glm::normalize(glm::cross(m_camera.direction, m_camera.UP));
            }

            if (m_camera.control && ImGui::IsKeyDown(ImGuiKey_E)) {
                position_offset += m_camera.UP;
            }

            if (m_camera.control && ImGui::IsKeyDown(ImGuiKey_Q)) {
                position_offset -= m_camera.UP;
            }

            if (glm::length(position_offset) > 0.0f) {
                m_camera.position += glm::normalize(position_offset) * MOVE_SPEED * m_camera.move_speed_multiplier * ImGui::GetIO().DeltaTime;
            }

            if (ImGui::IsKeyDown(ImGuiKey_Home)) {
                m_camera.position = m_camera.POSITION;
                m_camera.direction = m_camera.DIRECTION;
                m_camera.pitch = m_camera.PITCH;
                m_camera.yaw = m_camera.YAW;
            }
        }

        glm::vec3 direction {};
        direction.x = glm::cos(glm::radians(m_camera.yaw)) * glm::cos(glm::radians(m_camera.pitch));
        direction.y = glm::sin(glm::radians(m_camera.pitch));
        direction.z = glm::sin(glm::radians(m_camera.yaw)) * glm::cos(glm::radians(m_camera.pitch));
        m_camera.direction = glm::normalize(direction);

        renderer.m_camera.position_orientation(m_camera.position, m_camera.position + m_camera.direction, m_camera.UP);
    }

    void Editor::performance(renderer::Renderer& renderer) {
        if (ImGui::Begin("Performance")) {
            ImGui::Text("Frame Time: %.02f", renderer.m_performance.frame_time.count() * 1000.0);
            ImGui::Text("Draw Calls: %u", renderer.m_performance.draw_calls);
            ImGui::Text("Transform Updates: %u", renderer.m_performance.transform_updates);
        }

        ImGui::End();
    }

    void Editor::renderer(renderer::Renderer& renderer) {
        if (ImGui::Begin("Renderer")) {
            ImGui::ColorEdit3("Background Color", glm::value_ptr(renderer.m_background_color));

            char buffer[16] {};  // FIXME
            ImGui::InputText("Skybox", buffer, sizeof(buffer));

            {
                ImGui::SeparatorText("Fog");

                if (ImGui::Checkbox("Enabled", &renderer.m_environment.fog.enabled)) {
                    renderer.fog(renderer.m_environment.fog);
                }

                ImGui::DragFloat("Density", &renderer.m_environment.fog.density, 0.0005f, 0.0005f, 0.5f, "%.4f");
                ImGui::DragFloat("Gradient", &renderer.m_environment.fog.gradient, 0.005f, 0.5f, 10.0f);
                ImGui::ColorEdit3("Color", glm::value_ptr(renderer.m_environment.fog.color));
            }
        }

        ImGui::End();
    }

    void Editor::primitives_registry(renderer::Renderer&) {
        if (ImGui::Begin("Primitives Registry")) {
            primitives_object("Vertex Buffers", primitives_registry::primitives(primitives_registry::Primitive::VertexBuffer));
            primitives_object("Element Buffers", primitives_registry::primitives(primitives_registry::Primitive::ElementBuffer));
            primitives_object("Vertex Arrays", primitives_registry::primitives(primitives_registry::Primitive::VertexArray));
            primitives_object("Shaders", primitives_registry::primitives(primitives_registry::Primitive::Shader));
            primitives_object("Programs", primitives_registry::primitives(primitives_registry::Primitive::Program));
            primitives_object("Textures 2D", primitives_registry::primitives(primitives_registry::Primitive::Texture2D));
            primitives_object("Textures Cubemap", primitives_registry::primitives(primitives_registry::Primitive::TextureCubemap));
        }

        ImGui::End();
    }

    void Editor::primitives_object(const char* label, const auto& objects) {
        if (ImGui::TreeNode(label)) {
            for (const auto [i, object] : objects | std::views::enumerate) {
                ImGui::Text("%ld. %u", i + 1, object);
            }

            ImGui::TreePop();
        }
    }

    void Editor::scene_hierarchy(renderer::Renderer& renderer) {
        if (ImGui::Begin("Scene Hierarchy")) {
            scene_hierarchy_tree(renderer, renderer.root_node(), "");
        }

        ImGui::End();
    }

    void Editor::scene_hierarchy_tree(renderer::Renderer& renderer, scene::Node* node, std::string path) {
        const bool is_root = node->name().empty();

        path += node->name() + "/";

        static constexpr auto flags =
            ImGuiTreeNodeFlags_DrawLinesFull | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen;
        const bool selected = m_inspectable == node->shared_from_this();

        const bool open = ImGui::TreeNodeEx(
            path.c_str(),
            flags | (selected ? ImGuiTreeNodeFlags_Selected : ImGuiTreeNodeFlags_None),
            "%s",
            is_root ? "/" : node->name().data()
        );

        if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
            set_inspectable(node->shared_from_this(), node->shared_from_this()->name());
        }

        if (!is_root && ImGui::BeginPopupContextItem()) {
            ImGui::Text("%s", node->name().c_str());
            ImGui::Separator();

            if (ImGui::Button("Delete Node")) {
                renderer.m_task_manager.add_immediate_task([this, node = node->shared_from_this()] {
                    node->detach();
                    m_inspectable.reset();  // FIXME only reset this, if the selected node is dropped
                });

                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }

        if (open) {
            for (const auto& child : node->m_children | std::views::values) {
                scene_hierarchy_tree(renderer, child.get(), path);
            }

            node->editor_nodes(*this);

            ImGui::TreePop();
        }
    }

    void Editor::inspector(renderer::Renderer& renderer) {
        if (ImGui::Begin("Inspector")) {
            if (m_inspectable) {
                m_inspectable->editor_inspect(*this);
            }
        }

        ImGui::End();
    }

    void Editor::inspect(scene::RootNode* node) {
        ImGui::SeparatorText("Root");
    }

    void Editor::inspect(scene::ModelNode* node) {
        ImGui::SeparatorText("Model");

        if (input_text("Name", m_buffer_name, sizeof(m_buffer_name))) {
            node->m_name = m_buffer_name;
        }

        separator();

        auto translation = node->translation();
        auto rotation = node->rotation();
        auto scale = node->scale();

        if (ImGui::DragFloat3("Translation", glm::value_ptr(translation), 0.125f)) {
            node->translation(translation);
        }

        if (ImGui::DragFloat3("Rotation", glm::value_ptr(rotation), 1.0f, -180.0f, 180.0f)) {
            node->rotation(rotation);
        }

        if (ImGui::DragFloat3("Scale", glm::value_ptr(scale), 0.125f, 0.0f, 1000.0f)) {
            node->scale(scale);
        }
    }

    void Editor::inspect(scene::DirectionalLightNode* node) {
        ImGui::SeparatorText("Directional Light");

        if (input_text("Name", m_buffer_name, sizeof(m_buffer_name))) {
            node->m_name = m_buffer_name;
        }

        separator();

        ImGui::DragFloat3("Ambient", glm::value_ptr(node->directional_light.ambient_color), 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat3("Diffuse", glm::value_ptr(node->directional_light.diffuse_color), 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat3("Specular", glm::value_ptr(node->directional_light.specular_color), 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat3("Direction", glm::value_ptr(node->directional_light.direction), 0.1f);
    }

    void Editor::inspect(scene::PointLightNode* node) {
        ImGui::SeparatorText("Point Light");

        if (input_text("Name", m_buffer_name, sizeof(m_buffer_name))) {
            node->m_name = m_buffer_name;
        }

        separator();

        ImGui::DragFloat3("Ambient", glm::value_ptr(node->point_light.ambient_color), 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat3("Diffuse", glm::value_ptr(node->point_light.diffuse_color), 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat3("Specular", glm::value_ptr(node->point_light.specular_color), 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat3("Position", glm::value_ptr(node->point_light.position), 0.1f);
        ImGui::DragFloat("Linear", &node->point_light.linear, 0.001f, 0.0f, 1.0f);
        ImGui::DragFloat("Quadratic", &node->point_light.quadratic, 0.0001f, 0.0f, 1.0f);
    }

    void Editor::inspect(scene::SpotLightNode* node) {
        ImGui::SeparatorText("Spot Light");

        if (input_text("Name", m_buffer_name, sizeof(m_buffer_name))) {
            node->m_name = m_buffer_name;
        }

        separator();

        ImGui::DragFloat3("Ambient", glm::value_ptr(node->spot_light.ambient_color), 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat3("Diffuse", glm::value_ptr(node->spot_light.diffuse_color), 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat3("Specular", glm::value_ptr(node->spot_light.specular_color), 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat3("Position", glm::value_ptr(node->spot_light.position), 0.1f);
        ImGui::DragFloat3("Direction", glm::value_ptr(node->spot_light.direction), 0.1f);
        ImGui::DragFloat("Linear", &node->spot_light.linear, 0.001f, 0.0f, 1.0f);
        ImGui::DragFloat("Quadratic", &node->spot_light.quadratic, 0.0001f, 0.0f, 1.0f);
        ImGui::DragFloat("Cutoff Inner", &node->spot_light.cutoff_inner, 0.1f, 0.0f, 180.0f);
        ImGui::DragFloat("Cutoff Outer", &node->spot_light.cutoff_outer, 0.1f, 0.0f, 180.0f);
    }

    void Editor::inspect(ModelMesh* mesh) {
        ImGui::SeparatorText("Mesh");

        if (input_text("Name", m_buffer_name, sizeof(m_buffer_name))) {
            mesh->node->m_meshes[mesh->index].name = m_buffer_name;
        }

        separator();

        auto& material = *mesh->node->m_meshes[mesh->index].material;

        if (const auto iter = material.m_floats3.find("u_material.color_ambient"); iter != material.m_floats3.end()) {
            ImGui::DragFloat3("Ambient", glm::value_ptr(iter->second), 0.01f, 0.0f, 1.0f);
        }

        if (const auto iter = material.m_floats3.find("u_material.color_diffuse"); iter != material.m_floats3.end()) {
            ImGui::DragFloat3("Diffuse", glm::value_ptr(iter->second), 0.01f, 0.0f, 1.0f);
        }

        if (const auto iter = material.m_textures.find("u_material.texture_diffuse"); iter != material.m_textures.end()) {
            char buffer[16] {};  // FIXME
            ImGui::InputText("Diffuse", buffer, sizeof(buffer));
        }

        if (const auto iter = material.m_floats3.find("u_material.color_specular"); iter != material.m_floats3.end()) {
            ImGui::DragFloat3("Specular", glm::value_ptr(iter->second), 0.01f, 0.0f, 1.0f);
        }

        if (const auto iter = material.m_textures.find("u_material.texture_specular"); iter != material.m_textures.end()) {
            char buffer[16] {};  // FIXME
            ImGui::InputText("Specular", buffer, sizeof(buffer));
        }

        if (const auto iter = material.m_floats1.find("u_material.shininess"); iter != material.m_floats1.end()) {
            ImGui::DragFloat("Shininess", &iter->second, 1.0f, 1.0f, 512.0f);
        }

        constexpr const char* BLEND_MODE[] { "None", "Default", "Additive" };

        if (ImGui::BeginCombo("Blend Mode", BLEND_MODE[material.blend_mode])) {
            for (std::size_t i {}; i < std::size(BLEND_MODE); i++) {
                if (ImGui::Selectable(BLEND_MODE[i], material.blend_mode == i)) {
                    material.blend_mode = mesh::BlendMode(i);
                }
            }

            ImGui::EndCombo();
        }
    }

    void Editor::nodes(scene::ModelNode* node) {
        for (std::size_t i {}; const auto& mesh : node->m_meshes) {
            static constexpr auto flags =
                ImGuiTreeNodeFlags_DrawLinesFull | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Leaf;

            const bool selected = [this, node, i] {
                const auto inspectable = std::dynamic_pointer_cast<ModelMesh>(m_inspectable);

                if (inspectable) {
                    return inspectable->node == node->shared_from_this() && inspectable->index == i;
                }

                return false;
            }();

            const bool open = ImGui::TreeNodeEx(
                mesh.name.c_str(),
                flags | (selected ? ImGuiTreeNodeFlags_Selected : ImGuiTreeNodeFlags_None)
            );

            if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
                ModelMesh model_mesh;
                model_mesh.node = std::static_pointer_cast<scene::ModelNode>(node->shared_from_this());
                model_mesh.index = i++;

                set_inspectable(std::make_shared<ModelMesh>(model_mesh), model_mesh.node->m_meshes[model_mesh.index].name);
            }

            if (open) {
                ImGui::TreePop();
            }
        }
    }

    void Editor::world_origin(renderer::Renderer& renderer) {
        const float far = 100.0f;  // TODO

        renderer.add_debug_line(
            glm::vec3(-far, 0.0f, 0.0f),
            glm::vec3(far, 0.0f, 0.0f),
            glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)
        );

        renderer.add_debug_line(
            glm::vec3(0.0f, -far, 0.0f),
            glm::vec3(0.0f, far, 0.0f),
            glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)
        );

        renderer.add_debug_line(
            glm::vec3(0.0f, 0.0f, -far),
            glm::vec3(0.0f, 0.0f, far),
            glm::vec4(0.0f, 0.0f, 1.0f, 1.0f)
        );
    }

    void Editor::world_grid(renderer::Renderer& renderer) {
        static constexpr float CELL_SIZE = 1.0f;

        const float far = 100.0f;  // TODO

        for (float x {}; x < far; x += CELL_SIZE) {
            renderer.add_debug_line(
                glm::vec3(x + CELL_SIZE / 2.0f, 0.0f, -far),
                glm::vec3(x + CELL_SIZE / 2.0f, 0.0f, far),
                glm::vec4(0.5f, 0.5f, 0.5f, 1.0f)
            );
        }

        for (float x {}; x < far; x += CELL_SIZE) {
            renderer.add_debug_line(
                glm::vec3(-x - CELL_SIZE / 2.0f, 0.0f, -far),
                glm::vec3(-x - CELL_SIZE / 2.0f, 0.0f, far),
                glm::vec4(0.5f, 0.5f, 0.5f, 1.0f)
            );
        }

        for (float z {}; z < far; z += CELL_SIZE) {
            renderer.add_debug_line(
                glm::vec3(-far, 0.0f, z + CELL_SIZE / 2.0f),
                glm::vec3(far, 0.0f, z + CELL_SIZE / 2.0f),
                glm::vec4(0.5f, 0.5f, 0.5f, 1.0f)
            );
        }

        for (float z {}; z < far; z += CELL_SIZE) {
            renderer.add_debug_line(
                glm::vec3(-far, 0.0f, -z - CELL_SIZE / 2.0f),
                glm::vec3(far, 0.0f, -z - CELL_SIZE / 2.0f),
                glm::vec4(0.5f, 0.5f, 0.5f, 1.0f)
            );
        }
    }

    void Editor::world_bounding_box(renderer::Renderer& renderer) {
        if (!m_inspectable) {
            return;
        }

        bounding_box(renderer, std::dynamic_pointer_cast<ModelMesh>(m_inspectable).get()) ||
        bounding_box(renderer, std::dynamic_pointer_cast<scene::ModelNode>(m_inspectable).get());
    }

    bool Editor::bounding_box(renderer::Renderer& renderer, ModelMesh* mesh) {
        if (!mesh) {
            return false;
        }

        renderer.add_debug_aabb(mesh->node->m_meshes[mesh->index].aabb, mesh->node->m_world_transform);

        return true;
    }

    bool Editor::bounding_box(renderer::Renderer& renderer, scene::ModelNode* node) {
        if (!node) {
            return false;
        }

        aabb::Aabb aabb;
        bounding_box(node, aabb);

        renderer.add_debug_aabb(aabb, node->m_world_transform);

        return true;
    }

    void Editor::bounding_box(scene::ModelNode* node, aabb::Aabb& aabb) {
        {
            std::vector<aabb::Aabb> bounding_boxes;

            for (const scene::Mesh& mesh : node->m_meshes) {
                bounding_boxes.push_back(mesh.aabb);
            }

            aabb.min = glm::vec3(std::numeric_limits<float>::infinity());
            aabb.max = glm::vec3(-std::numeric_limits<float>::infinity());

            for (const auto& bounding_box : bounding_boxes) {
                aabb.min = glm::min(aabb.min, bounding_box.min);
                aabb.max = glm::max(aabb.max, bounding_box.max);
            }
        }

        for (const auto& child : node->m_children | std::views::values) {
            const auto model_node = std::dynamic_pointer_cast<scene::ModelNode>(child);

            if (model_node) {
                aabb::Aabb child_aabb;

                bounding_box(model_node.get(), child_aabb);

                const auto local_transform =
                    glm::translate(glm::identity<glm::mat4>(), model_node->m_local.translation) *
                    glm::mat4_cast(model_node->m_local.rotation) *
                    glm::scale(glm::identity<glm::mat4>(), model_node->m_local.scale);

                const glm::vec3 child_min = local_transform * glm::vec4(child_aabb.min, 1.0f);
                const glm::vec3 child_max = local_transform * glm::vec4(child_aabb.max, 1.0f);
                const glm::vec3 child_min_x = local_transform * glm::vec4(child_aabb.max.x, child_aabb.min.y, child_aabb.min.z, 1.0f);
                const glm::vec3 child_min_y = local_transform * glm::vec4(child_aabb.min.x, child_aabb.max.y, child_aabb.min.z, 1.0f);
                const glm::vec3 child_min_z = local_transform * glm::vec4(child_aabb.min.x, child_aabb.min.y, child_aabb.max.z, 1.0f);
                const glm::vec3 child_max_x = local_transform * glm::vec4(child_aabb.min.x, child_aabb.max.y, child_aabb.max.z, 1.0f);
                const glm::vec3 child_max_y = local_transform * glm::vec4(child_aabb.max.x, child_aabb.min.y, child_aabb.max.z, 1.0f);
                const glm::vec3 child_max_z = local_transform * glm::vec4(child_aabb.max.x, child_aabb.max.y, child_aabb.min.z, 1.0f);

                aabb.min = glm::min(aabb.min, child_min);
                aabb.min = glm::min(aabb.min, child_min_x);
                aabb.min = glm::min(aabb.min, child_min_y);
                aabb.min = glm::min(aabb.min, child_min_z);
                aabb.min = glm::min(aabb.min, child_max);
                aabb.min = glm::min(aabb.min, child_max_x);
                aabb.min = glm::min(aabb.min, child_max_y);
                aabb.min = glm::min(aabb.min, child_max_z);

                aabb.max = glm::max(aabb.max, child_max);
                aabb.max = glm::max(aabb.max, child_max_x);
                aabb.max = glm::max(aabb.max, child_max_y);
                aabb.max = glm::max(aabb.max, child_max_z);
                aabb.max = glm::max(aabb.max, child_min);
                aabb.max = glm::max(aabb.max, child_min_x);
                aabb.max = glm::max(aabb.max, child_min_y);
                aabb.max = glm::max(aabb.max, child_min_z);
            }
        }
    }

    void Editor::set_inspectable(std::shared_ptr<editor_common::Inspectable> inspectable, const std::string& name) {
        m_inspectable = std::move(inspectable);
        std::strncpy(m_buffer_name, name.c_str(), sizeof(m_buffer_name) - 1);
    }

    glm::mat4 Editor::ancestor_world_transform(std::shared_ptr<scene::Node> node) {
        while (true) {
            const auto parent = node->m_parent.lock();

            if (!parent) {
                return glm::identity<glm::mat4>();
            }

            const auto node_model = std::dynamic_pointer_cast<scene::ModelNode>(parent);

            if (!node_model) {
                node = std::move(parent);
                continue;
            }

            return node_model->m_world_transform;
        }
    }
}
