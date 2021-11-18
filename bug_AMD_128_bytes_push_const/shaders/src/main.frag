#version 450

#extension GL_ARB_separate_shader_objects: enable
#extension GL_ARB_shading_language_420pack: enable
#extension GL_GOOGLE_include_directive : enable

layout (location = 0) in vec2 frag_pos;

layout (push_constant) uniform push_constants
{
  uint bits[32];
} constants;

vec3 iResolution=vec3(vec2(1280.,720.),1.);

layout (location = 0) out vec4 out_color;

#include "main_image.glsl"

void main()
{
    vec4 uFragColor=vec4(0.);
    vec2 fragCoord=gl_FragCoord.xy;
    
    mainImage(uFragColor,fragCoord);
    out_color=uFragColor;
}
