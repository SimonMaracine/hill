vec3 fragment_position(mat4 transform, vec3 position) {
    return vec3(transform * vec4(position, 1.0));
}

vec3 fragment_normal(mat4 transform, vec3 normal) {
    return mat3(transpose(inverse(transform))) * normal;
}

vec4 vertex_position(mat4 projection_view, mat4 transform, vec3 position) {
    return projection_view * transform * vec4(position, 1.0);
}

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
