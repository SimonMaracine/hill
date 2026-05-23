#include "hill/shader_assembler.hpp"

#include <string_view>

#include "hill/utility.hpp"

namespace hill::shader_assembler {
    ShaderSourceBuilder& ShaderSourceBuilder::operator+=(std::string source) {
        return push(std::move(source));
    }

    ShaderSourceBuilder& ShaderSourceBuilder::operator+=(ShaderSourceBuilder other) {
        m_shader_sources.append_range(std::move(other.m_shader_sources));

        return *this;
    }

    ShaderSourceBuilder& ShaderSourceBuilder::push(std::string source, const char* end) {
        trim_whitespace(source);
        source += end;

        m_shader_sources.push_back(std::move(source));

        return *this;
    }

    bool ShaderSourceBuilder::is_whitespace(char character) {
        return character == ' ' || character == '\n' || character == '\t';
    }

    std::string_view ShaderSourceBuilder::trim_whitespace_prefix(std::string_view string) {
        std::size_t index {};

        for (; index < string.size(); index++) {
            if (!is_whitespace(string.at(index))) {
                break;
            }
        }

        string.remove_prefix(index);

        return string;
    }

    std::string_view ShaderSourceBuilder::trim_whitespace_suffix(std::string_view string) {
        std::size_t index = string.size();

        for (; index > 0; index--) {
            if (!is_whitespace(string.at(index - 1))) {
                break;
            }
        }

        string.remove_suffix(string.size() - index);

        return string;
    }

    void ShaderSourceBuilder::trim_whitespace(std::string& string) {
        auto view = std::string_view(string);
        view = trim_whitespace_prefix(view);
        view = trim_whitespace_suffix(view);
        string = view;
    }

    namespace source {
        static std::string prelude() {
            return "#version 430 core";
        }

        static std::string begin_main() {
            return "void main() {";
        }

        static std::string end_main() {
            return "}";
        }
    }

    namespace source::vertex {
        static std::string base_declarations() {
            return R"(
layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;

out vec3 v_fragment_position;
out vec3 v_fragment_normal;

uniform mat4 u_projection_view;
uniform mat4 u_transform;
            )";
        }

        static std::string base_computations() {
            return R"(
v_fragment_position = fragment_position(u_transform, a_position);
v_fragment_normal = fragment_normal(u_transform, a_normal);
gl_Position = vertex_position(u_projection_view, u_transform, a_position);
            )";
        }

        static std::string texture_coordinate_declarations() {
            return R"(
layout(location = 3) in vec2 a_texture_coordinate;

out vec2 v_texture_coordinate;
            )";
        }

        static std::string texture_coordinate_computations() {
            return R"(
 v_texture_coordinate = a_texture_coordinate;
            )";
        }
    }

    namespace source::fragment {
        static std::string base_declarations() {
            return R"(
in vec3 v_fragment_normal;
in vec3 v_fragment_position;

layout(location = 0) out vec4 o_color;

uniform vec3 u_view_position;
uniform DirectionalLight u_directional_light;
uniform Material u_material;
            )";
        }

        static std::string base_computations() {
            return R"(
PhongMaterial material;
material.color_ambient = u_material.color_ambient;
material.color_diffuse = u_material.color_diffuse;
material.color_specular = u_material.color_specular;
material.shininess = u_material.shininess;

const vec3 color = phong(u_directional_light, material, v_fragment_position, v_fragment_normal, u_view_position);
o_color = vec4(color, 1.0);
            )";
        }

        static std::string texture_coordinate_declarations() {
            return R"(
in vec2 v_texture_coordinate;
            )";
        }
    }

    void ShaderAssembler::initialize() {
        {
            utility::Buffer buffer;
            utility::read_file("shaders/vert_common.glsl", buffer);

            m_vertex_common_source = std::move(buffer);
        }

        {
            utility::Buffer buffer;
            utility::read_file("shaders/frag_common.glsl", buffer);

            m_fragment_common_source = std::move(buffer);
        }
    }

    std::shared_ptr<shader::Program> ShaderAssembler::assemble_program(renderer_common::ShaderFeatureSet shader_feature_set, const material::Material& material) const {
        auto vertex_shader = std::make_shared<shader::Shader>(shader::ShaderType::Vertex);
        vertex_shader->compile(assemble_vertex_shader(shader_feature_set).get());

        auto fragment_shader = std::make_shared<shader::Shader>(shader::ShaderType::Fragment);
        fragment_shader->compile(assemble_fragment_shader(shader_feature_set, material).get());

        const auto program = std::make_shared<shader::Program>();
        program->attach_shader(std::move(vertex_shader));
        program->attach_shader(std::move(fragment_shader));
        program->link();

        return program;
    }

    ShaderSourceBuilder ShaderAssembler::assemble_vertex_shader(renderer_common::ShaderFeatureSet shader_feature_set) const {
        ShaderSourceBuilder shader_source_builder;

        shader_source_builder += source::prelude();
        shader_source_builder += m_vertex_common_source;
        shader_source_builder += source::vertex::base_declarations();

        if (shader_feature_set & renderer_common::ShaderFeatureDiffuseMap) {
            shader_source_builder += source::vertex::texture_coordinate_declarations();
        }

        shader_source_builder += source::begin_main();
        shader_source_builder += source::vertex::base_computations();

        if (shader_feature_set & renderer_common::ShaderFeatureDiffuseMap) {
            shader_source_builder += source::vertex::texture_coordinate_computations();
        }

        shader_source_builder += source::end_main();

        return shader_source_builder;
    }

    ShaderSourceBuilder ShaderAssembler::assemble_fragment_shader(renderer_common::ShaderFeatureSet shader_feature_set, const material::Material& material) const {
        ShaderSourceBuilder shader_source_builder;

        shader_source_builder += source::prelude();
        shader_source_builder += m_fragment_common_source;
        shader_source_builder += assemble_material_declaraion(material);
        shader_source_builder += source::fragment::base_declarations();

        if (shader_feature_set & renderer_common::ShaderFeatureDiffuseMap) {
            shader_source_builder += source::fragment::texture_coordinate_declarations();
        }

        shader_source_builder += source::begin_main();
        shader_source_builder += source::fragment::base_computations();

        if (shader_feature_set & renderer_common::ShaderFeatureDiffuseMap) {

        }

        shader_source_builder += source::end_main();

        return shader_source_builder;
    }

    ShaderSourceBuilder ShaderAssembler::assemble_material_declaraion(const material::Material& material) {
        ShaderSourceBuilder shader_source_builder;

        shader_source_builder.push("struct Material {", "\n");

        if (const auto iter = material.m_floats3.find("u_material.color_ambient"); iter != material.m_floats3.end()) {
            shader_source_builder.push("material.color_ambient = u_material.color_ambient;", "\n");
        }

        if (const auto iter = material.m_floats3.find("u_material.color_diffuse"); iter != material.m_floats3.end()) {
            shader_source_builder.push("material.color_diffuse = u_material.color_diffuse;", "\n");
        }

        if (const auto iter = material.m_floats3.find("u_material.color_specular"); iter != material.m_floats3.end()) {
            shader_source_builder.push("material.color_specular = u_material.color_specular;", "\n");
        }

        if (const auto iter = material.m_floats1.find("u_material.shininess"); iter != material.m_floats1.end()) {
            shader_source_builder.push("material.shininess = u_material.shininess;", "\n");
        }

        if (const auto iter = material.m_textures.find("u_material.texture_diffuse"); iter != material.m_textures.end()) {
            shader_source_builder.push(
                R"(
material.color_ambient = vec3(texture(u_material.texture_diffuse));
material.color_diffuse = vec3(texture(u_material.texture_diffuse));
                )",
                "\n"
            );
        }

        shader_source_builder.push("};");

        return shader_source_builder;
    }

    ShaderSourceBuilder ShaderAssembler::assemble_material_computation(const material::Material& material) {
        ShaderSourceBuilder shader_source_builder;

        shader_source_builder.push("PhongMaterial material", "\n");

        if (const auto iter = material.m_floats3.find("u_material.color_ambient"); iter != material.m_floats3.end()) {
            shader_source_builder.push("vec3 color_ambient;", "\n");
        }

        if (const auto iter = material.m_floats3.find("u_material.color_diffuse"); iter != material.m_floats3.end()) {
            shader_source_builder.push("vec3 color_diffuse;", "\n");
        }

        if (const auto iter = material.m_floats3.find("u_material.color_specular"); iter != material.m_floats3.end()) {
            shader_source_builder.push("vec3 color_specular;", "\n");
        }

        if (const auto iter = material.m_floats1.find("u_material.shininess"); iter != material.m_floats1.end()) {
            shader_source_builder.push("float shininess;", "\n");
        }

        if (const auto iter = material.m_textures.find("u_material.texture_diffuse"); iter != material.m_textures.end()) {
            shader_source_builder.push("sampler2D texture_diffuse;", "\n");
        }

        shader_source_builder.push("};");

        return shader_source_builder;
    }
}
