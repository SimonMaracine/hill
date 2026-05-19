#include "hill/material.hpp"

#include <cstring>

#include "hill/renderer_command.hpp"

namespace hill::material {
    void MaterialDescription::add_uniform(Uniform uniform, std::string name) {
        switch (uniform) {
            case Uniform::Float1:
                m_uniforms_float1.insert(std::move(name));
                break;
            case Uniform::Float3:
                m_uniforms_float3.insert(std::move(name));
                break;
            case Uniform::Float16:
                m_uniforms_float16.insert(std::move(name));
                break;
        }
    }

    void MaterialDescription::add_texture(std::string name) {
        m_textures.insert(std::move(name));
    }

    Material::Material(const MaterialDescription& description) {
        m_program = description.m_program;

        unsigned int offset {};

        for (const auto& name : description.m_uniforms_float1) {
            Element element;
            element.type = Element::Float1;
            element.offset = offset;

            m_offsets[name] = element;
            offset += sizeof(float);
        }

        for (const auto& name : description.m_uniforms_float3) {
            Element element;
            element.type = Element::Float3;
            element.offset = offset;

            m_offsets[name] = element;
            offset += sizeof(glm::vec3);
        }

        for (const auto& name : description.m_uniforms_float16) {
            Element element;
            element.type = Element::Float16;
            element.offset = offset;

            m_offsets[name] = element;
            offset += sizeof(glm::mat4);
        }

        for (const auto& name : description.m_textures) {
            Element element;
            element.type = Element::Texture;
            element.offset = offset;

            m_offsets[name] = element;
            offset += sizeof(Texture);
        }

        m_size = offset;
        m_data = std::make_unique<unsigned char[]>(m_size);
    }

    void Material::upload_data() {
        for (const auto& [name, element] : m_offsets) {
            switch (element.type) {
                case Element::Float1: {
                    float value {};
                    std::memcpy(&value, m_data.get() + element.offset, sizeof(value));

                    m_program->upload_uniform_float1(name, value);

                    break;
                }
                case Element::Float3: {
                    glm::vec3 value {};
                    std::memcpy(&value, m_data.get() + element.offset, sizeof(value));

                    m_program->upload_uniform_float3(name, value);

                    break;
                }
                case Element::Float16: {
                    glm::mat4 value {};
                    std::memcpy(&value, m_data.get() + element.offset, sizeof(value));

                    m_program->upload_uniform_float16(name, value);

                    break;
                }
                case Element::Texture: {
                    Texture value;
                    std::memcpy(&value, m_data.get() + element.offset, sizeof(value));

                    m_program->upload_uniform_int1(name, int(value.unit));
                    renderer_command::bind_texture_2d(value.texture, value.unit);

                    break;
                }
            }
        }
    }

    void Material::set_float1(const std::string& name, float value) {
        const Element& element = m_offsets.at(name);
        std::memcpy(m_data.get() + element.offset, &value, sizeof(value));
    }

    void Material::set_float3(const std::string& name, glm::vec3 value) {
        const Element& element = m_offsets.at(name);
        std::memcpy(m_data.get() + element.offset, &value, sizeof(value));
    }

    void Material::set_float16(const std::string& name, const glm::mat4& value) {
        const Element& element = m_offsets.at(name);
        std::memcpy(m_data.get() + element.offset, &value, sizeof(value));
    }

    void Material::set_texture(const std::string& name, unsigned int texture, unsigned int unit) {
        Texture value;
        value.texture = texture;
        value.unit = unit;

        const Element& element = m_offsets.at(name);
        std::memcpy(m_data.get() + element.offset, &value, sizeof(value));
    }
}
