#version 430 core

in vec3 v_position;

layout(location = 0) out vec4 o_color;

uniform samplerCube u_texture;  // Value is 0 by default

void main() {
    o_color = texture(u_texture, v_position);
}
