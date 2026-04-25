#pragma once

#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

namespace hill::camera {
    class Camera {
    public:
        void projection(int width, int height, float fov, float near, float far) {
            m_projection = glm::perspective(glm::radians(fov), float(width) / float(height), near, far);
        }

        void position_orientation(glm::vec3 position, glm::vec3 at, glm::vec3 up) {
            m_view = glm::lookAt(position, at, up);
        }

        void position_rotation(glm::vec3 position, glm::vec3 rotation) {
            glm::mat4 matrix = glm::identity<glm::mat4>();
            matrix = glm::translate(matrix, position);
            matrix = glm::rotate(matrix, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
            matrix = glm::rotate(matrix, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
            matrix = glm::rotate(matrix, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

            m_view = glm::inverse(matrix);
        }

        void update_projection_view() {
            m_projection_view = m_projection * m_view;
        }

        const glm::mat4& view() const { return m_view; }
        const glm::mat4& projection() const { return m_projection; }
        const glm::mat4& projection_view() const { return m_projection_view; }
    private:
        glm::mat4 m_view = glm::identity<glm::mat4>();
        glm::mat4 m_projection = glm::identity<glm::mat4>();

        // Cached matrix
        glm::mat4 m_projection_view = glm::identity<glm::mat4>();
    };
}
