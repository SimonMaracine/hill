struct DirectionalLight {
    vec3 direction;
    vec3 color_ambient;
    vec3 color_diffuse;
    vec3 color_specular;
};

struct PhongMaterial {
    vec3 color_ambient;
    vec3 color_diffuse;
    vec3 color_specular;
    float shininess;
};

vec3 phong(DirectionalLight directional_light, PhongMaterial material, vec3 fragment_position, vec3 fragment_normal, vec3 view_position) {
    fragment_normal = normalize(fragment_normal);
    const vec3 light_direction = normalize(-directional_light.direction);
    const vec3 view_direction = normalize(view_position - fragment_position);
    const vec3 reflection_direction = reflect(-light_direction, fragment_normal);

    // Ambient
    const vec3 ambient_light = directional_light.color_ambient * material.color_ambient;

    // Diffuse
    const float diffuse_strength = max(dot(fragment_normal, light_direction), 0.0);
    const vec3 diffuse_light = diffuse_strength * directional_light.color_diffuse * material.color_diffuse;

    // Specular
    const float specular_strength = pow(max(dot(view_direction, reflection_direction), 0.0), material.shininess);
    const vec3 specular_light = specular_strength * directional_light.color_specular * material.color_specular;

    return ambient_light + diffuse_light + specular_light;
}
