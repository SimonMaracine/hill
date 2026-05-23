vec3 fragment_position(mat4 transform, vec3 position) {
    return vec3(transform * vec4(position, 1.0));
}

vec3 fragment_normal(mat4 transform, vec3 normal) {
    return mat3(transpose(inverse(transform))) * normal;
}

vec4 vertex_position(mat4 projection_view, mat4 transform, vec3 position) {
    return projection_view * transform * vec4(position, 1.0);
}
