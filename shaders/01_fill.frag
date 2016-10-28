#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (set = 2, binding = 1) uniform sampler2D main_texture;

layout (location = 0) in vec3 vs_nrm;
layout (location = 1) in vec2 vs_tex;

layout (location = 0) out vec4 out_color;
layout (location = 1) out vec4 out_normal;

void main()
{
    vec3 nrm = normalize(vs_nrm);
    out_color = texture(main_texture, vs_tex);
    out_normal = vec4(vs_nrm, 1.0f);
}
