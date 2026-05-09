#pragma once

#include <memory>

#include <glm/glm.hpp>

#include "hill/primitives/shader.hpp"

namespace hill::renderer {
    class Renderer;
}

namespace hill::material {
    class Material {
    public:
        Material() = default;
        explicit Material(std::shared_ptr<shader::Program> program)
            : m_program(std::move(program)) {}

        virtual ~Material() = default;

        Material(const Material&) = default;
        Material& operator=(const Material&) = default;
        Material(Material&&) = default;
        Material& operator=(Material&&) = default;
    protected:
        std::shared_ptr<shader::Program> m_program;
    private:
        virtual void upload_data() = 0;

        friend class renderer::Renderer;
    };

    class MaterialBasic : public Material {
    public:
        using Material::Material;

        glm::vec3 ambient_color {0.6f};
        glm::vec3 diffuse_color {0.6f};
        glm::vec3 specular_color {0.6f};
        float shininess = 32.0f;
    private:
        void upload_data() override {
            m_program->upload_uniform_float3("u_material.ambient_color", ambient_color);
            m_program->upload_uniform_float3("u_material.diffuse_color", diffuse_color);
            m_program->upload_uniform_float3("u_material.specular_color", specular_color);
            m_program->upload_uniform_float1("u_material.shininess", shininess);
        }
    };
}
