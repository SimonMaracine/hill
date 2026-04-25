#pragma once

#include <vector>
#include <string>
#include <memory>
#include <unordered_set>

#include <glm/glm.hpp>

#include "hill/primitives/common.hpp"
#include "hill/error.hpp"

namespace hill::shader {
    struct ShaderError : error::Error {
        using Error::Error;
    };

    struct CompileError : ShaderError {
        using ShaderError::ShaderError;
    };

    struct LinkError : ShaderError {
        using ShaderError::ShaderError;
    };

    enum class ShaderType {
        Vertex,
        Fragment
    };

    class Shader {
    public:
        explicit Shader(ShaderType shader_type);
        ~Shader();

        Shader(const Shader&) = delete;
        Shader& operator=(const Shader&) = delete;
        Shader(Shader&&) = delete;
        Shader& operator=(Shader&&) = delete;

        unsigned int id() const { return m_shader; }

        void compile(const std::string& source) const;
        void compile(const std::vector<std::string>& sources) const;
    private:
        bool compilation_successful() const;
        std::string info_log() const;

        unsigned int m_shader {};
    };

    class Program {
    public:
        Program();
        ~Program();

        Program(const Program&) = delete;
        Program& operator=(const Program&) = delete;
        Program(Program&&) = delete;
        Program& operator=(Program&&) = delete;

        unsigned int id() const { return m_program; }

        void attach_shader(std::shared_ptr<Shader> shader);
        void detach_shader(const std::shared_ptr<Shader>& shader);
        void detach_shader(unsigned int shader);

        void link() const;

        void use() const;
        void unuse() const;

        void upload_uniform_float3(const std::string& name, glm::vec3 value) const;
        void upload_uniform_float16(const std::string& name, const glm::mat4& value) const;
    private:
        bool link_successful() const;
        std::string info_log() const;

        unsigned int m_program {};
        std::unordered_set<std::shared_ptr<Shader>, common::IdHash<Shader>> m_shaders;
    };
}
