
// Remember to change #define fbo_size to size of FBO, dont use textureSize diferent behavior

//#define fbo_size vec2(800.,450.)
#define fbo_size vec2(1280.,720.)

#define is_true  (iTime>-1.+min(iTime,0.))
#define is_false (iTime< 0.+min(iTime,0.))

#define is_true2  (min(iTime,0.)>-2.)
#define is_false2 (min(iTime,0.)<-1.)

vec4 bug_func()
{
    vec2 uvc = vec2(min(0.,iTime))+0.05;
    
    if (is_true && is_false) { //false
      uvc = vec2(0.5 * (uvc / uvc.x + 1.))*vec2(min(0.,iTime)); // uvc / uvc.x needed to make bug work
    }else
    if (is_false) { //false
      uvc = vec2(0.5 * (uvc / uvc.x + 1.))*vec2(min(0.,iTime)); // uvc / uvc.x needed to make bug work
    }
    return vec4(uvc,uvc);
}

void mainImage(out vec4 fragColor, in vec2 fragCoord) {
    
    // different result
    vec4 Color=textureLod(iChannel0,fragCoord.xy/iResolution.xy,0.); // uncomment one of this
    //vec4 Color=texelFetch(iChannel0,ivec2(fragCoord.xy),0); // uncomment one of this
    
    vec2 fragFloor=floor(fragCoord);

    if (fragCoord.y<1.) {
      if (fragCoord.x<2.) {
              
            } else if (fragCoord.x<3.) { 
                Color=texture(iChannel0,vec2(2.5,0.5)*1./fbo_size); // bug
                //Color=texture(iChannel0,vec2(2.5,0.5)*1./iResolution.xy); // fix 1
                //Color=textureLod(iChannel0,vec2(2.5,0.5)*1./fbo_size,0.); // fix 2
                //Color=texelFetch(iChannel0,ivec2(2.5,0.5),0); // fix 3
            }
    } else if is_true2 {
        Color=bug_func();
    } else if is_false2 {
        Color=bug_func();
    }
    fragColor=Color;
}

