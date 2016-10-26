#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (set = 2, binding = 0) uniform Material
{
    vec4 diffuse;
};

layout (set = 3, binding = 0) uniform Light
{
    vec4 diffuse;
    vec4 position;
    vec4 direction;
} light;

layout (set = 2, binding = 1) uniform sampler2D main_texture;

layout (location = 0) in vec3 vs_nrm;
layout (location = 1) in vec2 vs_tex;
layout (location = 2) in vec3 vs_light_dir;

layout (location = 0) out vec4 out_color;
layout (location = 1) out vec4 out_normal;

void main()
{
    vec3 nrm = normalize(vs_nrm);
    vec3 light_dir = normalize(vs_light_dir);
    float diffuse_term = max(dot(nrm, light_dir), 0.0f);
    out_color = texture(main_texture, vs_tex) * (light.diffuse * diffuse_term + vec4(0.2f, 0.2f, 0.2f, 1.0f));
    out_normal = vec4(vs_nrm, 1.0f);
}
