vec3 fragment_position(mat4 transform, vec3 position) {
    return vec3(transform * vec4(position, 1.0));
}

vec3 fragment_normal(mat4 transform, vec3 normal) {
    return mat3(transpose(inverse(transform))) * normal;
}

vec4 vertex_position(mat4 projection, mat4 view, mat4 transform, vec3 position) {
    return projection * view * transform * vec4(position, 1.0);
}

float fog_visibility(mat4 view, mat4 transform, vec3 position, float density, float gradient) {
    const vec4 position_eye = view * transform * vec4(position, 1.0);
    const float distance = length(position_eye.xyz);

    return clamp(exp(-pow(distance * density, gradient)), 0.0, 1.0);
}

// -------------------------------------------------------------------

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;

out vec3 v_fragment_position;
out vec3 v_fragment_normal;

uniform mat4 u_projection;
uniform mat4 u_view;
uniform mat4 u_transform;

#ifdef META_FEATURE_TEXTURE_COORDINATES
layout(location = 3) in vec2 a_texture_coordinate;
out vec2 v_texture_coordinate;
#endif

#ifdef FEATURE_FOG
out float v_fog_visibility;
uniform float u_fog_density;
uniform float u_fog_gradient;
#endif

void main() {
    v_fragment_position = fragment_position(u_transform, a_position);
    v_fragment_normal = fragment_normal(u_transform, a_normal);
    gl_Position = vertex_position(u_projection, u_view, u_transform, a_position);

#ifdef META_FEATURE_TEXTURE_COORDINATES
    v_texture_coordinate = a_texture_coordinate;
#endif

#ifdef FEATURE_FOG
    v_fog_visibility = fog_visibility(u_view, u_transform, a_position, u_fog_density, u_fog_gradient);
#endif
}
