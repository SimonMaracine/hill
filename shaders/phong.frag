struct BaseColoredLight {
    vec3 ambient;  // TODO make single color
    vec3 diffuse;
    vec3 specular;
};

struct DirectionalLight {
    BaseColoredLight color;
    vec3 direction;
};

struct PointLight {
    BaseColoredLight color;
    vec3 position;
    float linear;
    float quadratic;
};

struct PointLights {
    PointLight elements[8];
    uint count;
};

struct PhongMaterial {
    vec3 color_ambient;
    vec3 color_diffuse;
    vec3 color_specular;
    float shininess;
};

void phong(
    BaseColoredLight light,
    PhongMaterial material,
    vec3 fragment_position,
    vec3 fragment_normal,
    vec3 view_position,
    vec3 light_direction,
    vec3 view_direction,
    vec3 reflection_direction,
    out vec3 ambient_light,
    out vec3 diffuse_light,
    out vec3 specular_light
) {
    // Ambient
    ambient_light = light.ambient * material.color_ambient;

    // Diffuse
    const float diffuse_strength = max(dot(fragment_normal, light_direction), 0.0);
    diffuse_light = diffuse_strength * light.diffuse * material.color_diffuse;

    // Specular
    const float specular_strength = pow(max(dot(view_direction, reflection_direction), 0.0), material.shininess);
    specular_light = specular_strength * light.specular * material.color_specular;
}

vec3 phong(DirectionalLight directional_light, PointLights point_lights, PhongMaterial material, vec3 fragment_position, vec3 fragment_normal, vec3 view_position) {
    fragment_normal = normalize(fragment_normal);
    const vec3 view_direction = normalize(view_position - fragment_position);


    vec3 directional_light_contribution;
    {
        vec3 ambient_light;
        vec3 diffuse_light;
        vec3 specular_light;

        const vec3 light_direction = normalize(-directional_light.direction);
        const vec3 reflection_direction = reflect(-light_direction, fragment_normal);

        phong(
            directional_light.color,
            material,
            fragment_position,
            fragment_normal,
            view_position,
            light_direction,
            view_direction,
            reflection_direction,
            ambient_light,
            diffuse_light,
            specular_light
        );

        directional_light_contribution = ambient_light + diffuse_light + specular_light;
    }

    vec3 point_lights_contribution;
    for (uint i = 0; i < point_lights.count; i++) {
        vec3 ambient_light;
        vec3 diffuse_light;
        vec3 specular_light;

        const vec3 light_direction = normalize(point_lights.elements[i].position - fragment_position);
        const vec3 reflection_direction = reflect(-light_direction, fragment_normal);

        const float distance = length(point_lights.elements[i].position - fragment_position);
        const float attenuation = 1.0 / (1.0 + point_lights.elements[i].linear * distance + point_lights.elements[i].quadratic * distance * distance);

        phong(
            point_lights.elements[i].color,
            material,
            fragment_position,
            fragment_normal,
            view_position,
            light_direction,
            view_direction,
            reflection_direction,
            ambient_light,
            diffuse_light,
            specular_light
        );

        point_lights_contribution += (ambient_light + diffuse_light + specular_light) * attenuation;
    }

    return directional_light_contribution + point_lights_contribution;
}

struct Material {
#ifndef FEATURE_DIFFUSE_MAP
    vec3 color_ambient;
    vec3 color_diffuse;
#else
    sampler2D texture_diffuse;
#endif

#ifndef FEATURE_SPECULAR_MAP
    vec3 color_specular;
#else
    sampler2D texture_specular;
#endif

    float shininess;
};

// -------------------------------------------------------------------

in vec3 v_fragment_normal;
in vec3 v_fragment_position;

layout(location = 0) out vec4 o_color;

uniform vec3 u_view_position;
uniform DirectionalLight u_directional_light;
uniform PointLights u_point_lights;
uniform Material u_material;

#ifdef META_FEATURE_TEXTURE_COORDINATES
in vec2 v_texture_coordinate;
#endif

PhongMaterial get_material() {
    PhongMaterial material;

#ifndef FEATURE_DIFFUSE_MAP
    material.color_ambient = u_material.color_ambient;
    material.color_diffuse = u_material.color_diffuse;
#else
    material.color_ambient = vec3(texture(u_material.texture_diffuse, v_texture_coordinate));
    material.color_diffuse = vec3(texture(u_material.texture_diffuse, v_texture_coordinate));
#endif

#ifndef FEATURE_SPECULAR_MAP
    material.color_specular = u_material.color_specular;
#else
    material.color_specular = vec3(texture(u_material.texture_specular, v_texture_coordinate));
#endif

    material.shininess = u_material.shininess;

    return material;
}

void main() {
    const vec3 color = phong(u_directional_light, u_point_lights, get_material(), v_fragment_position, v_fragment_normal, u_view_position);
    o_color = vec4(color, 1.0);
}
