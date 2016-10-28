#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 nrm;
layout (location = 2) in vec3 tng;
layout (location = 3) in vec2 tex;

layout (set = 0, binding = 0) uniform PerCamera
{
    mat4 vp;
};

layout (set = 1, binding = 0) uniform PerModel
{
    mat4 world;
};

layout(location = 0) out vec3 vs_nrm;
layout(location = 1) out vec2 vs_tex;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
    mat3 normal_transform = mat3(world);
    vs_nrm = normal_transform * nrm;
    vs_tex = tex;
    gl_Position = vp * world * vec4(pos, 1);
}
