#include "hill/editor.hpp"

#include <ranges>
#include <cstring>

#include <imgui.h>

#include "glm/gtx/matrix_decompose.hpp"
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

    void Editor::initialize() {
        auto& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    }

    void Editor::uninitialize() {
        m_inspectable.reset();
    }

    void Editor::update(renderer::Renderer& renderer) {
        ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

        ImGui::ShowDemoWindow();

        performance(renderer);
        Editor::renderer(renderer);
        primitives_registry(renderer);
        scene_hierarchy(renderer);
        inspector(renderer);
    }

    void Editor::update_camera(renderer::Renderer& renderer, windowing_system::WindowingSystem& windowing_system) {
        static constexpr float MOVE_SPEED = 10.0f;
        static constexpr float LOOK_SPEED = 7.0f;

        if (!ImGui::GetIO().WantCaptureMouse) {
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
                windowing_system.grab_mouse();
                m_camera.control = true;
            }

            if (ImGui::IsMouseReleased(ImGuiMouseButton_Right)) {
                windowing_system.ungrab_mouse();
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
                position_offset += m_camera.front;
            }

            if (m_camera.control && ImGui::IsKeyDown(ImGuiKey_S)) {
                position_offset -= m_camera.front;
            }

            if (m_camera.control && ImGui::IsKeyDown(ImGuiKey_A)) {
                position_offset -= glm::normalize(glm::cross(m_camera.front, m_camera.up));
            }

            if (m_camera.control && ImGui::IsKeyDown(ImGuiKey_D)) {
                position_offset += glm::normalize(glm::cross(m_camera.front, m_camera.up));
            }

            if (m_camera.control && ImGui::IsKeyDown(ImGuiKey_E)) {
                position_offset += m_camera.up;
            }

            if (m_camera.control && ImGui::IsKeyDown(ImGuiKey_Q)) {
                position_offset -= m_camera.up;
            }

            if (glm::length(position_offset) > 0.0f) {
                m_camera.position += glm::normalize(position_offset) * MOVE_SPEED * m_camera.move_speed_multiplier * ImGui::GetIO().DeltaTime;
            }
        }

        glm::vec3 direction {};
        direction.x = glm::cos(glm::radians(m_camera.yaw)) * glm::cos(glm::radians(m_camera.pitch));
        direction.y = glm::sin(glm::radians(m_camera.pitch));
        direction.z = glm::sin(glm::radians(m_camera.yaw)) * glm::cos(glm::radians(m_camera.pitch));
        m_camera.front = glm::normalize(direction);

        renderer.m_camera.position_orientation(m_camera.position, m_camera.position + m_camera.front, m_camera.up);
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
        }

        ImGui::End();
    }

    void Editor::primitives_registry(renderer::Renderer&) {
        if (ImGui::Begin("Primitives Registry")) {
            primitives_object("Vertex Buffers", primitives_registry::Registry::get().primitives(primitives_registry::Primitive::VertexBuffer));
            primitives_object("Element Buffers", primitives_registry::Registry::get().primitives(primitives_registry::Primitive::ElementBuffer));
            primitives_object("Vertex Arrays", primitives_registry::Registry::get().primitives(primitives_registry::Primitive::VertexArray));
            primitives_object("Shaders", primitives_registry::Registry::get().primitives(primitives_registry::Primitive::Shader));
            primitives_object("Programs", primitives_registry::Registry::get().primitives(primitives_registry::Primitive::Program));
            primitives_object("Textures 2D", primitives_registry::Registry::get().primitives(primitives_registry::Primitive::Texture2D));
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
            scene_hierarchy_tree(renderer.root_node(), "");
        }

        ImGui::End();
    }

    void Editor::scene_hierarchy_tree(scene::Node* node, std::string path) {
        using namespace std::string_literals;

        path += node->name().data() + "/"s;

        static constexpr auto flags = ImGuiTreeNodeFlags_DrawLinesFull | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
        const bool selected = m_inspectable == node->shared_from_this();

        const bool open = ImGui::TreeNodeEx(
            path.c_str(),
            flags | (selected ? ImGuiTreeNodeFlags_Selected : ImGuiTreeNodeFlags_None),
            "%s",
            node->name().empty() ? "/" : node->name().data()
        );

        if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
            set_inspectable(node->shared_from_this(), node->shared_from_this()->name());
        }

        if (open) {
            for (const auto& child : node->m_children | std::views::values) {
                scene_hierarchy_tree(child.get(), path);
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

        ImGui::DragFloat3("Direction", glm::value_ptr(node->directional_light.direction), 0.1f);
        ImGui::DragFloat3("Ambient", glm::value_ptr(node->directional_light.ambient_color), 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat3("Diffuse", glm::value_ptr(node->directional_light.diffuse_color), 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat3("Specular", glm::value_ptr(node->directional_light.specular_color), 0.01f, 0.0f, 1.0f);
    }

    void Editor::inspect(ModelMesh* mesh) {
        ImGui::SeparatorText("Mesh");

        if (input_text("Name", m_buffer_name, sizeof(m_buffer_name))) {
            mesh->node->m_meshes[mesh->index].name = m_buffer_name;
        }

        separator();

        material_basic(dynamic_cast<material::MaterialBasic*>(mesh->node->m_meshes[mesh->index].material.get()));
    }

    void Editor::nodes(scene::ModelNode* node) {
        for (std::size_t i {}; const auto& mesh : node->m_static_meshes) {
            static constexpr auto flags = ImGuiTreeNodeFlags_DrawLinesFull | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Leaf;

            const bool selected = [this, node, i] {
                const auto inspectable = std::dynamic_pointer_cast<ModelMesh>(m_inspectable);

                if (inspectable) {
                    return inspectable->node == node->shared_from_this() && inspectable->index == i;
                }

                return false;
            }();

            const bool open = ImGui::TreeNodeEx(
                mesh->name.c_str(),
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

    bool Editor::material_basic(material::MaterialBasic* material) {
        if (!material) {
            return false;
        }

        ImGui::DragFloat3("Ambient", glm::value_ptr(material->ambient_color), 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat3("Diffuse", glm::value_ptr(material->diffuse_color), 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat3("Specular", glm::value_ptr(material->specular_color), 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat("Shininess", &material->shininess, 1.0f, 1.0f, 512.0f);

        return true;
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
