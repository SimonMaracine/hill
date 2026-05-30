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

struct SpotLight {
    BaseColoredLight color;
    vec3 position;
    vec3 direction;
    float linear;
    float quadratic;
    float cutoff_inner;
    float cutoff_outer;
};

struct SpotLights {
    SpotLight elements[8];
    uint count;
};

struct PhongMaterial {
    vec3 color_ambient;
    vec3 color_diffuse;
    vec3 color_specular;
    float shininess;
};

struct PhongResult {
    vec3 ambient_light;
    vec3 diffuse_light;
    vec3 specular_light;
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
    out PhongResult result
) {
    // Ambient
    result.ambient_light = light.ambient * material.color_ambient;

    // Diffuse
    const float diffuse_strength = max(dot(fragment_normal, light_direction), 0.0);
    result.diffuse_light = diffuse_strength * light.diffuse * material.color_diffuse;

    // Specular
    const float specular_strength = pow(max(dot(view_direction, reflection_direction), 0.0), material.shininess);
    result.specular_light = specular_strength * light.specular * material.color_specular;
}

vec3 phong(
    DirectionalLight directional_light,
    PointLights point_lights,
    SpotLights spot_lights,
    PhongMaterial material,
    vec3 fragment_position,
    vec3 fragment_normal,
    vec3 view_position
) {
    fragment_normal = normalize(fragment_normal);
    const vec3 view_direction = normalize(view_position - fragment_position);

    vec3 directional_light_contribution;
    {
        const vec3 light_direction = normalize(-directional_light.direction);
        const vec3 reflection_direction = reflect(-light_direction, fragment_normal);

        PhongResult result;

        phong(
            directional_light.color,
            material,
            fragment_position,
            fragment_normal,
            view_position,
            light_direction,
            view_direction,
            reflection_direction,
            result
        );

        directional_light_contribution = result.ambient_light + result.diffuse_light + result.specular_light;
    }

    vec3 point_lights_contribution;
    for (uint i = 0; i < point_lights.count; i++) {
        const PointLight light = point_lights.elements[i];

        const vec3 light_direction = normalize(light.position - fragment_position);
        const vec3 reflection_direction = reflect(-light_direction, fragment_normal);

        const float distance = length(light.position - fragment_position);
        const float attenuation = 1.0 / (1.0 + light.linear * distance + light.quadratic * distance * distance);

        PhongResult result;

        phong(
            light.color,
            material,
            fragment_position,
            fragment_normal,
            view_position,
            light_direction,
            view_direction,
            reflection_direction,
            result
        );

        point_lights_contribution += (result.ambient_light + result.diffuse_light + result.specular_light) * attenuation;
    }

    vec3 spot_lights_contribution;
    for (uint i = 0; i < spot_lights.count; i++) {
        const SpotLight light = spot_lights.elements[i];

        const vec3 light_direction = normalize(light.position - fragment_position);
        const vec3 reflection_direction = reflect(-light_direction, fragment_normal);

        const float distance = length(light.position - fragment_position);
        const float attenuation = 1.0 / (1.0 + light.linear * distance + light.quadratic * distance * distance);

        const float theta = dot(light_direction, normalize(-light.direction));
        const float epsilon = light.cutoff_inner - light.cutoff_outer;
        const float intensity = clamp((theta - light.cutoff_outer) / epsilon, 0.0, 1.0);

        PhongResult result;

        phong(
            light.color,
            material,
            fragment_position,
            fragment_normal,
            view_position,
            light_direction,
            view_direction,
            reflection_direction,
            result
        );

        spot_lights_contribution += (result.ambient_light + result.diffuse_light + result.specular_light) * attenuation * intensity;
    }

    return directional_light_contribution + point_lights_contribution + spot_lights_contribution;
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
uniform SpotLights u_spot_lights;
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
    const vec3 color = phong(u_directional_light, u_point_lights, u_spot_lights, get_material(), v_fragment_position, v_fragment_normal, u_view_position);
    o_color = vec4(color, 1.0);
}
