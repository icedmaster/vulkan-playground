#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (set = 0, binding = 0) uniform PerCamera
{
    mat4 vp;
    mat4 inv_vp;
};

layout (set = 1, binding = 0) uniform sampler2D albedo_texture;
layout (set = 1, binding = 1) uniform sampler2D normal_texture;
layout (set = 1, binding = 2) uniform sampler2D depth_texture;

layout (location = 0) in vec2 vs_tex;

layout (location = 0) out vec4 out_color;

void main()
{
    float depth = texture(depth_texture, vs_tex).x;
    vec4 pos_cs = vec4(vs_tex * 2.0f - 1.0f, depth, 1.0f);
    vec4 pos = inv_vp * pos_cs;
    vec3 pos_ws = pos.xyz / pos.w;
    out_color = texture(albedo_texture, vs_tex);
}
