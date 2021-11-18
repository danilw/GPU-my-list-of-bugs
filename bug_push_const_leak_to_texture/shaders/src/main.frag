#version 450

#extension GL_ARB_separate_shader_objects: enable
#extension GL_ARB_shading_language_420pack: enable
#extension GL_GOOGLE_include_directive : enable

layout (location = 0) in vec2 frag_pos;

layout (set = 0, binding = 0) uniform sampler2D u_Channel0;

layout (push_constant) uniform push_constants
{
  vec4 u_Mouse;
  vec4 u_Date;
  bvec2 u_Mouse_lr;
  vec2 u_Resolution;
  bool u_debugdraw;
  int pCustom;
  float u_Time;
  float u_TimeDelta;
  int u_Frame;
} constants;

vec3 iResolution=vec3(constants.u_Resolution,1.);
float iTime=constants.u_Time;
float iTimeDelta=constants.u_TimeDelta;
int iFrame=constants.u_Frame;
vec4 iMouse=constants.u_Mouse;
vec4 iDate=constants.u_Date;
bool is_debugdraw=constants.u_debugdraw;
bool is_pause=bool(constants.pCustom-(constants.pCustom/10)*10);
bool main_image_srgb=bool((constants.pCustom/10)*10-(constants.pCustom/100)*100);

#define iChannel0 u_Channel0

layout (location = 0) out vec4 out_color;

#include "main_image.glsl"

void main()
{
    vec4 uFragColor=vec4(0.);
    vec2 fragCoord=gl_FragCoord.xy;
    fragCoord.y=iResolution.y-fragCoord.y; // shadertoy v(y)-flip main_image
    
    mainImage(uFragColor,fragCoord);
    out_color=uFragColor;
    if(main_image_srgb)out_color.rgb = ((exp2(out_color.rgb)-1.0)-out_color.rgb*0.693147)*3.258891;
}
