#pragma once

#include <memory>
#include <unordered_set>

#include "hill/primitives/shader.hpp"
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
        MaterialDescription() = default;
        explicit MaterialDescription(std::shared_ptr<shader::Program> program)
            : m_program(std::move(program)) {}

        enum class Uniform {
            Float1,
            Float3,
            Float16
        };

        void add_uniform(Uniform uniform, std::string name);
        void add_texture(std::string name);
    private:
        std::shared_ptr<shader::Program> m_program;

        std::unordered_set<std::string> m_uniforms_float1;
        std::unordered_set<std::string> m_uniforms_float3;
        std::unordered_set<std::string> m_uniforms_float16;
        std::unordered_set<std::string> m_textures;

        friend class Material;
        friend class renderer::Renderer;
    };

    class Material {
    public:
        explicit Material(const MaterialDescription& description);

        void upload_data();

        void set_float1(const std::string& name, float value);
        void set_float3(const std::string& name, glm::vec3 value);
        void set_float16(const std::string& name, const glm::mat4& value);
        void set_texture(const std::string& name, unsigned int texture, unsigned int unit);
    private:
        struct Element {
            enum Type : unsigned int {
                Float1,
                Float3,
                Float16,
                Texture
            } type {};

            unsigned int offset {};
        };

        struct Texture {
            unsigned int texture {};
            unsigned int unit {};
        };

        std::shared_ptr<shader::Program> m_program;

        std::unique_ptr<unsigned char[]> m_data;
        std::size_t m_size {};
        std::unordered_map<std::string, Element> m_offsets;
    };
}
