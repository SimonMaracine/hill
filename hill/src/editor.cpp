#include "hill/editor.hpp"

#include <ranges>

#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>

#include "hill/renderer.hpp"
#include "hill/primitives_registry.hpp"
#include "hill/scene.hpp"

namespace hill::editor {
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

        ImGui::Begin("Test");
        ImGui::ColorEdit3("Background Color", renderer.m_background_color);
        ImGui::End();

        performance(renderer);
        primitives_registry(renderer);
        scene_hierarchy(renderer);
        inspector(renderer);
    }

    void Editor::update_camera(renderer::Renderer& renderer) {
        static constexpr float MOVE_SPEED = 5.0f;
        static constexpr float LOOK_SPEED = 7.0f;

        if (!ImGui::GetIO().WantCaptureMouse) {
            if (ImGui::IsMouseDragging(ImGuiMouseButton_Right)) {
                const ImVec2 delta = ImGui::GetIO().MouseDelta;
                m_camera.yaw += -delta.x * LOOK_SPEED * ImGui::GetIO().DeltaTime;
                m_camera.pitch += delta.y * LOOK_SPEED * ImGui::GetIO().DeltaTime;
            }
        }

        m_camera.pitch = glm::min(m_camera.pitch, 89.0f);
        m_camera.pitch = glm::max(m_camera.pitch, -89.0f);

        if (!ImGui::GetIO().WantCaptureKeyboard) {
            if (ImGui::IsKeyDown(ImGuiKey_LeftShift)) {
                m_camera.move_speed_multiplier = 5.0f;
            } else {
                m_camera.move_speed_multiplier = 1.0f;
            }

            if (ImGui::IsKeyDown(ImGuiKey_W)) {
                m_camera.position += m_camera.front * MOVE_SPEED * m_camera.move_speed_multiplier * ImGui::GetIO().DeltaTime;
            } else if (ImGui::IsKeyDown(ImGuiKey_S)) {
                m_camera.position -= m_camera.front * MOVE_SPEED * m_camera.move_speed_multiplier * ImGui::GetIO().DeltaTime;
            } else if (ImGui::IsKeyDown(ImGuiKey_A)) {
                m_camera.position -= glm::normalize(glm::cross(m_camera.front, m_camera.up)) * MOVE_SPEED * m_camera.move_speed_multiplier * ImGui::GetIO().DeltaTime;
            } else if (ImGui::IsKeyDown(ImGuiKey_D)) {
                m_camera.position += glm::normalize(glm::cross(m_camera.front, m_camera.up)) * MOVE_SPEED * m_camera.move_speed_multiplier * ImGui::GetIO().DeltaTime;
            } else if (ImGui::IsKeyDown(ImGuiKey_E)) {
                m_camera.position += m_camera.up * MOVE_SPEED * m_camera.move_speed_multiplier * ImGui::GetIO().DeltaTime;
            } else if (ImGui::IsKeyDown(ImGuiKey_Q)) {
                m_camera.position -= m_camera.up * MOVE_SPEED * m_camera.move_speed_multiplier * ImGui::GetIO().DeltaTime;
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
            ImGui::Text("Frame time: %.02f", renderer.m_frame_time.count() * 1000.0);
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
            m_inspectable = node->shared_from_this();
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
        ImGui::DragFloat3("Position", glm::value_ptr(node->position), 0.1f);
        ImGui::DragFloat3("Rotation", glm::value_ptr(node->rotation), 1.0f, 0.0f, 360.0f);
        ImGui::DragFloat3("Scale", glm::value_ptr(node->scale), 0.01f, 0.0f, 100.0f);
    }

    void Editor::inspect(scene::DirectionalLightNode* node) {
        ImGui::SeparatorText("Directional Light");
        ImGui::DragFloat3("Direction", glm::value_ptr(node->directional_light.direction), 0.1f);
        ImGui::DragFloat3("Ambient", glm::value_ptr(node->directional_light.ambient_color), 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat3("Diffuse", glm::value_ptr(node->directional_light.diffuse_color), 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat3("Specular", glm::value_ptr(node->directional_light.specular_color), 0.01f, 0.0f, 1.0f);
    }

    void Editor::inspect(ModelMesh* mesh) {
        ImGui::SeparatorText("Mesh");
        material_basic(dynamic_cast<material::MaterialBasic*>(mesh->node->m_objects.at(mesh->index).material.get()));
    }

    void Editor::nodes(scene::ModelNode* node) {
        for (std::size_t i {}; const auto& mesh : node->m_meshes) {
            static constexpr auto flags = ImGuiTreeNodeFlags_DrawLinesFull | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Leaf;
            // const bool selected = m_inspectable == node->shared_from_this();

            const bool open = ImGui::TreeNodeEx(
                mesh->name.c_str(),
                flags /*| (selected ? ImGuiTreeNodeFlags_Selected : ImGuiTreeNodeFlags_None)*/
            );

            if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
                ModelMesh model_mesh;
                model_mesh.node = std::static_pointer_cast<scene::ModelNode>(node->shared_from_this());
                model_mesh.index = i++;

                m_inspectable = std::make_shared<ModelMesh>(model_mesh);
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
        ImGui::DragFloat("Shininess", &material->shininess, 1.0f, 1.0f, 256.0f);

        return true;
    }
}
