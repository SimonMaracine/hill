#pragma once

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <variant>

#include "hill/primitives/shader.hpp"
#include "hill/primitives/texture2d.hpp"
#include "hill/glm.h++"

namespace hill::renderer {
    class Renderer;
}

namespace hill::material {
    // class Material {
    // public:
    //     Material() = default;
    //     explicit Material(std::shared_ptr<shader::Program> program)
    //         : m_program(std::move(program)) {}
    //
    //     virtual ~Material() = default;
    //
    //     Material(const Material&) = default;
    //     Material& operator=(const Material&) = default;
    //     Material(Material&&) = default;
    //     Material& operator=(Material&&) = default;
    // protected:
    //     std::shared_ptr<shader::Program> m_program;
    // private:
    //     virtual void upload_data() = 0;
    //
    //     friend class renderer::Renderer;
    // };
    //
    // class MaterialBasic : public Material {
    // public:
    //     using Material::Material;
    //
    //     glm::vec3 ambient_color {0.6f};
    //     glm::vec3 diffuse_color {0.6f};
    //     glm::vec3 specular_color {0.6f};
    //     float shininess = 32.0f;
    // private:
    //     void upload_data() override {
    //         m_program->upload_uniform_float3("u_material.ambient_color", ambient_color);
    //         m_program->upload_uniform_float3("u_material.diffuse_color", diffuse_color);
    //         m_program->upload_uniform_float3("u_material.specular_color", specular_color);
    //         m_program->upload_uniform_float1("u_material.shininess", shininess);
    //     }
    // };

    class Material;

    class MaterialDescription {
    public:
        void add_uniform(std::string name, float value);
        void add_uniform(std::string name, glm::vec3 value);
        void remove_uniform(const std::string& name);

        void add_texture(std::string name);
        void remove_texture(const std::string& name);
    private:
        using Uniform = std::variant<float, glm::vec3>;

        std::unordered_map<std::string, Uniform> m_uniforms;
        std::unordered_set<std::string> m_textures;

        friend class Material;
    };

    class Material {
    public:
        explicit Material(const MaterialDescription& description);

        void upload_data();

        void set_float1(const std::string& name, float value);
        void set_float3(const std::string& name, glm::vec3 value);
        void set_texture(const std::string& name, std::shared_ptr<texture2d::Texture2D> texture, unsigned int unit);
    private:
        std::unordered_map<std::string, float> m_floats1;
        std::unordered_map<std::string, glm::vec3> m_floats3;
        std::unordered_map<std::string, std::pair<std::shared_ptr<texture2d::Texture2D>, unsigned int>> m_textures;

        std::shared_ptr<shader::Program> m_program;

        friend class renderer::Renderer;
    };
}
