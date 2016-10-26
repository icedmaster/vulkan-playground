#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (set = 0, binding = 0) uniform sampler2D main_texture;

layout (location = 0) in vec2 vs_tex;

layout (location = 0) out vec4 out_color;

void main()
{
    out_color = texture(main_texture, vs_tex);
}
