
# GPU-my-list-of-bugs

**what is it** - list of bugs I found writing shaders, mostly shader bugs.

**Maybe this is my code bug** or/and shader bugs, but **this code has no Vulkan Validation errors**, and shader code very simple with no obvious bugs (all arrays range clamped and all variables initialized)

All of this "bugs" works(can be reproduced) in Khronos Vulkan Examples, this why I think - this is not my code bugs (atleast not Vulkan code side).

___

To select GPU use launch option `--gpu X` where `X` is ID of GPU 0-1-2-3 etc. Used GPU printed to terminal output on application launch.

___

# Bugs (this repo code)

### bug_AMD_128_bytes_push_const

I send **128 bytes push_const to fragment shader**. (bug works with similar behavior when push_const send to Vertex shader also, in Fragment mode bug better visible)

**[Shadertoy shader](https://www.shadertoy.com/view/3sXyW2)** - bug does not work on shadertoy, bug works only when bits are push_const that impossible in WebGL and WebGPU.

Building: (on Windows use MSVS and open cmake file there)

```
cd bug_AMD_128_bytes_push_const
mkdir build
cmake ../
make
./Vkexample
```

**Bug result** - on AMD 128 bytes push_const (almost)always bugged in shader, only solution to dodge bug - use less than 64 bytes push_const. 
Works same on Mesa AMD Linux drivers, and AMD Windows drivers.

**Expected result** - no noise on screen.

**Test** - on AMD you should see noise on application start, or try to resize window.

Nvidia and AMD result image: 

![AMD_128](https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/vk_amd_128pc.png)

Related bugreports - [mesa/-/issues/5656](https://gitlab.freedesktop.org/mesa/mesa/-/issues/5656)

Download bin build *Warning exe/bin file you may not trust this*  - [bug_AMD_128_bytes_push_const.zip](https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/bug_AMD_128_bytes_push_const.zip)

### bug_push_const_leak_to_texture

When **sampler2D have *no mipmaps* but *has linear filter* and shader *has push_const*** - functions `texture` has some unpredictable result. (woks only in this specific shader(shader made from other large shader where I found this bug)).
Bug does not work when shader does not have push_constants.

**[Shadertoy shader](https://www.shadertoy.com/view/sltGWj)** - bug work on Shadertoy only in OpenGL mode `chrome.exe --use-angle=gl` and only when FBO has mipmaps. 
But in OpenGL this is "not a bug" or atleast it can be explained, Nviida behavior in OpenGL you can see on video below it still very weird(numbers blinking without any consistency), but it also can be explained - mipmap generation is inconsistant for texture(FBO) with non pow2 siz, I think this is reason for OpenGL behvior in this shader.

Building: (on Windows use MSVS and open cmake file there)

```
cd bug_push_const_leak_to_texture
mkdir build
cmake ../
make
./Vkexample_fbo
```

**Bug result** - anything that not static 0.0 or 5.0 in numbers on screen.
Works same on all drivers and platforms (Linux/Windows).

**Expected result** - 0.0 on initial launch and 5.0 on resize.

**Test** - bug may not work from start of application, try to resize window (resize to bigger window size).

Works on AMD and Nvidia(with not same behavior), I still not sure is it my code bug or not, so idk, **youtube video link** of this bug on Nvidia:

[![youtube_mip](https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/yt_vkmip.png)](https://youtu.be/OcpBVHMb88M)

This is not important/critical bug and very rate, so I dont want to bugreport it. (and reproducing this bug in Khoronos Vulkan examples way too complicated, no one gonna read it)

Download bin build *Warning exe/bin file you may not trust this* - [bug_mip_inconsistent.zip](https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/bug_mip_inconsistent.zip)

___

___

# Other bugs in shaders related to Vulkan/DX11/OpenGL, list of my shader bugs

Most of this bugs works in Webbrowser (Shadertoy). 

___

## Debug:

### To **test this bugs in Vulkan**:

Use **[vulkan-shadertoy-launcher Release](https://github.com/danilw/vulkan-shadertoy-launcher/releases)** and copy-paste Shadertoy shader to `shaders/shadertoy/*.glsl` files.

### To **test in OpenGL**:

Use Linux or in Windows just launch Webbroser with disabled Angle `chrome.exe --use-angle=gl`

### To test shaders result in CPU-shader emulation:

*Use swiftshader* (works on every platform but this is *not best option*) 

`google-chrome --incognito --use-gl=swiftshader-webgl`

Many of listed shaders will may crash *swiftshader* or have completely wrong result because bugs in swiftshader.

**Use Mesa LLVM** (llvmpipe) OpenGL emulation (**best option**) 

`Xephyr -br -ac -noreset -screen 1280x720 :10&`

And launch firefox there `DISPLAY=:10 XDG_SESSION_TYPE=x11 firefox`

It will render WebGL on CPU llvmpipe driver. (do not launch chrome this way, chrome will use swiftshader instead of llvmpipe). To see that *llvmpipe* used - `DISPLAY=:10 glxinfo | grep OpenGL | grep string`

___

## List of shader bugs (Shadertoy links, look *in comments* on Shadertoy page for *screenshot of bug*)

[BUG ANGLE dFd broken on break](https://www.shadertoy.com/view/wdVyWD) - Angle bug

[BUG Angle 20x slowdown](https://www.shadertoy.com/view/flSSDV) - DX11 [FXC bug](https://bugs.chromium.org/p/chromium/issues/detail?id=1238461)

[BUG Chrome or Nvidia compiling](https://www.shadertoy.com/view/fsSSzd) - Nvidia OpenGL compiler bug

[BUG Nvidia OpenGL compiler bug](https://www.shadertoy.com/view/NsdXRs) - Nvidia OpenGL compiler bug

[BUG Nvidia OpenGL arrays wrong](https://www.shadertoy.com/view/7tjGRW) - only Nvidia OpenGL

[BUG Nvidia const to array](https://www.shadertoy.com/view/NslGR4) - Nvidia OpenGL and Vulkan driver shader compiler bug

[BUG Rand/Hash pre-calculated](https://www.shadertoy.com/view/wdXGW8) - I think this is not expected, related to blog post [Float precision on GPU, bugs/features](https://arugl.medium.com/float-precision-on-gpu-bugs-features-178ddd030f)

[BUG Vulkan AMD crash](https://www.shadertoy.com/view/wdfcDX) - old bug, [fixed already](https://community.amd.com/thread/250887)

[BUG Vulkan AMD loop bug](https://www.shadertoy.com/view/tsXyDH) - old bug, [fixed already](https://community.amd.com/message/2964120)

[BUG Vulkan Nvidia noise bug](https://www.shadertoy.com/view/ttjcRW) - [fixed already](https://forums.developer.nvidia.com/t/vulkan-shader-bug-can-someone-confirm-is-this-only-my-bug/140392), but very weird - basically all shaders that use fract and mod was bugged in Nvidia Vulkan for 5 years and no one noticed...

[BUG Vulkan uint32 128 bytes](https://www.shadertoy.com/view/3sXyW2) - related to [GPU-my-list-of-bugs repository](https://github.com/danilw/GPU-my-list-of-bugs)

[BUG floatBitsToUint comp vs real](https://www.shadertoy.com/view/tlfBRB) - related to blog post [Float precision on GPU, bugs/features](https://arugl.medium.com/float-precision-on-gpu-bugs-features-178ddd030f)

[BUG OpenGL Nvidia high GPU usage](https://www.shadertoy.com/view/tdfGWS) - Nvidia OpenGL compiler bug

[BUG smoothstep( 1, 0 ,0) specs](https://www.shadertoy.com/view/tdf3zf) - `smoothsep` on GPU does not follow specs, when on CPU it does.



## *Not a Bug*, its expected behavior but it still weird

[BUG 32-bit float precision](https://www.shadertoy.com/view/sllXW8) - related to blog post [Float precision on GPU, bugs/features](https://arugl.medium.com/float-precision-on-gpu-bugs-features-178ddd030f)

[BUG 32-bit float precision test](https://www.shadertoy.com/view/ftXXW4) - one of bugs related to *length of normalized vector not equal 1*

[BUG GPU driver unroll prediction](https://www.shadertoy.com/view/NlXXWS) - using break in loop adds precision to operation, expected because shader compiler compile shader diferently for each case.

[BUG GPU precision never 0](https://www.shadertoy.com/view/ftXSWB) - related to blog post [Float precision on GPU, bugs/features](https://arugl.medium.com/float-precision-on-gpu-bugs-features-178ddd030f)

[BUG Mip inconsistent Vulkan/Ogl](https://www.shadertoy.com/view/sltGWj) - related to [GPU-my-list-of-bugs repository](https://github.com/danilw/GPU-my-list-of-bugs)

[BUG Nvidia Vulkan UB behavior](https://www.shadertoy.com/view/fsc3RM) - weird UB behavior that result not same image depends on GPU, Vulkan only

[BUG Vulkan not equal to OpenGL](https://www.shadertoy.com/view/slsXzs) - weird UB behavior on GPU, on same GPU result not equal in OpenGL and Vulkan. Can be expected its UB.

[BUG cubemap rayDir to fragCoord](https://www.shadertoy.com/view/7l33W2) - related to [Cubemap GLSL shader debug functions](https://arugl.medium.com/cubemap-glsl-shader-debug-functions-3f4c659e7833)

[BUG normalize bugs](https://www.shadertoy.com/view/7lt3Rl) - related to [Cubemap GLSL shader debug functions](https://arugl.medium.com/cubemap-glsl-shader-debug-functions-3f4c659e7833)









