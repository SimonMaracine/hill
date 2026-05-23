#include "hill/material.hpp"

#include "hill/renderer_command.hpp"

namespace hill::material {
    void MaterialDescription::add_uniform(std::string name, float value) {
        m_uniforms[std::move(name)] = value;
    }

    void MaterialDescription::add_uniform(std::string name, glm::vec3 value) {
        m_uniforms[std::move(name)] = value;
    }

    void MaterialDescription::remove_uniform(const std::string& name) {
        m_uniforms.erase(name);
    }

    void MaterialDescription::add_texture(std::string name) {
        m_textures.insert(std::move(name));
    }

    void MaterialDescription::remove_texture(const std::string& name) {
        m_textures.erase(name);
    }

    Material::Material(const MaterialDescription& description) {
        for (const auto& [name, value] : description.m_uniforms) {
            switch (value.index()) {
                case 0:
                    m_floats1[name] = std::get<0>(value);
                    break;
                case 1:
                    m_floats3[name] = std::get<1>(value);
                    break;
            }
        }

        for (const auto& name : description.m_textures) {
            m_textures[name] = {};
        }
    }

    void Material::upload_data() {
        for (const auto& [name, value] : m_floats1) {
            m_program->upload_uniform_float1(name, value);
        }

        for (const auto& [name, value] : m_floats3) {
            m_program->upload_uniform_float3(name, value);
        }

        for (const auto& [name, value] : m_textures) {
            m_program->upload_uniform_int1(name, int(value.second));
            renderer_command::bind_texture_2d(value.first->id(), value.second);
        }
    }

    float Material::get_float1(const std::string& name) const {
        return m_floats1.at(name);
    }

    glm::vec3 Material::get_float3(const std::string& name) const {
        return m_floats3.at(name);
    }

    Texture Material::get_texture(const std::string& name) const {
        return m_textures.at(name);
    }

    void Material::set_float1(const std::string& name, float value) {
        m_floats1.at(name) = value;
    }

    void Material::set_float3(const std::string& name, glm::vec3 value) {
        m_floats3.at(name) = value;
    }

    void Material::set_texture(const std::string& name, Texture texture) {
        m_textures.at(name) = std::move(texture);
    }
}
