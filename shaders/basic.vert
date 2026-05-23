#version 430 core

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;

out vec3 v_fragment_position;
out vec3 v_fragment_normal;

uniform mat4 u_projection_view;
uniform mat4 u_transform;

void main() {
    v_fragment_position = vec3(u_transform * vec4(a_position, 1.0));
    v_fragment_normal = mat3(transpose(inverse(u_transform))) * a_normal;
    gl_Position = u_projection_view * u_transform * vec4(a_position, 1.0);
}
