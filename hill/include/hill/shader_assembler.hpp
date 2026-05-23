#pragma once

#include <memory>
#include <string>

#include "hill/primitives/shader.hpp"
#include "hill/renderer_common.hpp"
#include "hill/material.hpp"

namespace hill::shader_assembler {
    class ShaderSourceBuilder {
    public:
        ShaderSourceBuilder& operator+=(std::string source);
        ShaderSourceBuilder& operator+=(ShaderSourceBuilder other);
        ShaderSourceBuilder& push(std::string source, const char* end = "\n\n");

        const std::vector<std::string>& get() const { return m_shader_sources; }
    private:
        static bool is_whitespace(char character);
        static std::string_view trim_whitespace_prefix(std::string_view string);
        static std::string_view trim_whitespace_suffix(std::string_view string);
        static void trim_whitespace(std::string& string);

        std::vector<std::string> m_shader_sources;
    };

    class ShaderAssembler {
    public:
        void initialize();
        std::shared_ptr<shader::Program> assemble_program(renderer_common::ShaderFeatureSet shader_feature_set, const material::Material& material) const;
    private:
        ShaderSourceBuilder assemble_vertex_shader(renderer_common::ShaderFeatureSet shader_feature_set) const;
        ShaderSourceBuilder assemble_fragment_shader(renderer_common::ShaderFeatureSet shader_feature_set, const material::Material& material) const;
        static ShaderSourceBuilder assemble_material_declaraion(const material::Material& material);
        static ShaderSourceBuilder assemble_material_computation(const material::Material& material);

        std::string m_vertex_common_source;
        std::string m_fragment_common_source;
    };
}
