#pragma once

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <variant>

#include "hill/primitives/shader.hpp"
#include "hill/primitives/texture_2d.hpp"
#include "hill/glm.h++"

namespace hill::renderer {
    class Renderer;
}

namespace hill::editor {
    class Editor;
}

namespace hill::shader_assembler {
    class ShaderAssembler;
}

namespace hill::material {
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

    using Texture = std::pair<std::shared_ptr<texture_2d::Texture2D>, unsigned int>;

    class Material {
    public:
        explicit Material(const MaterialDescription& description);

        void upload_data();

        float get_float1(const std::string& name) const;
        glm::vec3 get_float3(const std::string& name) const;
        Texture get_texture(const std::string& name) const;

        void set_float1(const std::string& name, float value);
        void set_float3(const std::string& name, glm::vec3 value);
        void set_texture(const std::string& name, Texture texture);
    private:
        std::unordered_map<std::string, float> m_floats1;
        std::unordered_map<std::string, glm::vec3> m_floats3;
        std::unordered_map<std::string, Texture> m_textures;

        std::shared_ptr<shader::Program> m_program;

        friend class renderer::Renderer;
        friend class editor::Editor;
        friend class shader_assembler::ShaderAssembler;
    };
}
