#version 430 core

layout(location = 0) in vec3 a_position;

out vec3 v_position;

uniform mat4 u_projection_view;

void main() {
    v_position = a_position;
    const vec4 position = u_projection_view * vec4(a_position, 1.0);
    gl_Position = position.xyww;
}
