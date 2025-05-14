
# GPU-my-list-of-bugs

**what is it** - list of bugs I found writing shaders, mostly shader bugs.

## Scroll down to table with screenshots

### Contact: [**Join discord server**](https://discord.gg/JKyqWgt)

___

### Some information related to GPU-stuff can be found it my blog:

- [Medium blog link](https://arugl.medium.com/)
- [Github mirror of blog](https://danilw.github.io/blog/)

___

## Debug:

*To test in Vulkan (Windows/Linux) - you can use my shader launcher (it compatible with most of shadertoy shaders)*:\
*[Vulkan Shadertoy Launcher](https://github.com/danilw/vulkan-shadertoy-launcher/releases) - follow instructions there.*

### On Windows:

**OpenGL**:

`chrome.exe --use-angle=gl --incognito`

**Vulkan**:

`chrome.exe --use-angle=vulkan --enable-features=Vulkan,DefaultANGLEVulkan,VulkanFromANGLE --incognito`

*Swiftshader*: (do not use it super bugged and broken - there no way to know is it bug in swiftshader on in your code, it also crash)
`chrome.exe --use-gl=swiftshader-webgl --incognito`

### On Linux:

**OpenGL**:

`/usr/bin/google-chrome-stable`

To force Nvidia GPU over integrated (when display connected to integrated, and Nvidia is second):
```
export __NV_PRIME_RENDER_OFFLOAD=1
export __GLX_VENDOR_LIBRARY_NAME=nvidia
/usr/bin/google-chrome-stable
```

*To use zink - https://wiki.archlinux.org/title/OpenGL#OpenGL_over_Vulkan_(Zink) `MESA_LOADER_DRIVER_OVERRIDE=zink`*

**Vulkan**:

```
/usr/bin/google-chrome-stable --use-angle=vulkan --enable-features=Vulkan,DefaultANGLEVulkan,VulkanFromANGLE --incognito
```

To force Nvidia for Vulkan - way too complicated - Chrome by default should use discrete when launch in Vulkan mode.\
*(Chrome may not work in Vulkan mode for some reason - especially on new(2023+) Nvidia GPUs)*

**OpenGL llvmpipe (CPU emulation)**: (very stable, useful)

*Firefox in its own x11 server*:\
`Xephyr -br -ac -noreset -screen 1280x720 :10&`

And launch firefox there `DISPLAY=:10 XDG_SESSION_TYPE=x11 firefox`\
(do not launch chrome this way, chrome will use swiftshader instead of llvmpipe)\
To see *llvmpipe* used - `DISPLAY=:10 glxinfo | grep OpenGL | grep string`

*Without x11 server*:\
`LIBGL_ALWAYS_SOFTWARE=1 __GLX_VENDOR_LIBRARY_NAME=mesa VK_ICD_FILENAMES=/usr/share/vulkan/icd.d/lvp_icd.i686.json:/usr/share/vulkan/icd.d/lvp_icd.x86_64.json firefox`

**Vulkan llvmpipe (CPU emulation)**: (~stable)

In Linux install package something like `libvulkan_lvp - Mesa vulkan driver for LVP` (use package search in your Linux).\
And select `--gpu=0` in my [**vulkan-shadertoy-launcher**](https://github.com/danilw/vulkan-shadertoy-launcher/releases) launch line option.\
GPU should be detected as:\
`Vulkan GPU - CPU: llvmpipe (LLVM 13.0.0, 256 bits)...`

___

## List of shader bugs:

### Shadertoy links, click on Screenshot previews to see full image.

| Screenshot, click to open | Bug link |
|-------------|------------|
| <img src="https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/7tjGRW.png" width="650" height="auto" /> | [BUG C Undefined Behavior (a+=b) as argument](https://www.shadertoy.com/view/7tjGRW) (array and matX indexing). </br> This bug-shader was working only in Nvidia OpenGL(2025 570 driver seems fixed). </br> **Can be considered as "not a bug" because UB** line 31 `ext_3[return_zero()] += -2147483648` as function argument, so in unknown what will be first `+=` or passing as parameter. </br> And [bug for `pow(z -= 5., 2.);`](https://www.shadertoy.com/view/tc33RS). </br> Similar case but also `pow2` replaced with `((z -= 5.)*(z -= 5.))` </br> I saw this mistakes many times for last year - so move it to bug for showing common mistake. |
| <img src="https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/wdVyWD.png" width="650" height="auto" /> | [BUG ANGLE dFd broken on break](https://www.shadertoy.com/view/wdVyWD) - Angle bug |
| <img src="https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/NslGR4.png" width="650" height="auto" /> | [BUG Nvidia const to array](https://www.shadertoy.com/view/NslGR4) (array index bug) - Nvidia OpenGL and *Vulkan(were fixed in 2024)* driver shader compiler bug, [Other Nvidia OpenGL bugs in array](https://forums.developer.nvidia.com/t/opengl3-out-in-mat4-broken-on-many-nvidia-videocards-in-vertex-shader-shader-code-included/145921) - it still there even in [OpenGL compute shaders (Nvidia forum link)](https://forums.developer.nvidia.com/t/glsl-compiler-bug-passing-inout-mat4-to-a-function-in-compute-shader/325805) |
| <img src="https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/NtlyWn.png" width="650" height="auto" /> | [BUG Vulkan AMD array indexing](https://www.shadertoy.com/view/NtlyWn) (array index bug) - AMD *Vulkan driver bug (were fixed in 2024)*, [bugreport](https://community.amd.com/t5/opengl-vulkan/vulkan-amd-arrays-or-precompiling-spirv-bug/m-p/517283#M4172). <br>And **if you combine this code with Nvidia Vulkan array indexing bug - you can have shader that bugged on both Nvidia and AMD at same time** (yeee super cool). |
| <img src="https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/wdXGW8.png" width="650" height="auto" /> | [BUG Rand/Hash pre-calculated](https://www.shadertoy.com/view/wdXGW8) - I think this is not expected, related to blog post [Float precision on GPU, bugs/features](https://arugl.medium.com/float-precision-on-gpu-bugs-features-178ddd030f) |
| <img src="https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/tlfBRB.png" width="650" height="auto" /> | [BUG floatBitsToUint comp vs real](https://www.shadertoy.com/view/tlfBRB) - related to blog post [Float precision on GPU, bugs/features](https://arugl.medium.com/float-precision-on-gpu-bugs-features-178ddd030f) |
| <img src="https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/DdySWW.png" width="650" height="auto" /> | [BUG floatBits uintBits](https://www.shadertoy.com/view/DdySWW) - different version of same bug above - shader pre-compiler change behavior of pre-compiled values when used `uintBitsToFloat`. |
| <img src="https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/wfs3zr.png" width="650" height="auto" /> | [BUG uintBitsToFloat bits texture](https://www.shadertoy.com/view/wfs3zr) - one more of similar to 2 above - I step on it again spending time debuggine when I used texture to read compressed data. |
| **sin hash instability** | read blog - [**Hash Noise stability in GPU Shaders**](https://arugl.medium.com/hash-noise-in-gpu-shaders-210188ac3a3e) |
| <img src="https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/stK3WG.png" width="650" height="auto" /> | [BUG Vulkan Nvidia sin hash](https://www.shadertoy.com/view/stK3WG) - sin hash broken in Vulkan on Nvidia, when in OpenGL everything fine. |
| <img src="https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/7ddfzr.jpg" width="650" height="auto" /> | [BUG sin not same on AMD/Nvidia](https://www.shadertoy.com/view/7ddfzr) - similar as "sin hash broken" above, but in not-noise shader, in fractal shader sin making different visual image. |
| <img src="https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/7ddfzr_mod2pi.jpg" width="650" height="auto" /> | [BUG mod2pi does not fix sin/cos AMD/Nvidia](https://www.shadertoy.com/view/7ddfzr) - same shader as above, uncomment first two lines with `define`, obvious idea to fix sin inconsistensy just `mod(x,2*PI)` but it not just "not fix" it change pattern on Nvidia where it was working, on Nvidia pattern on left side with define will be different compare to no define. |
| --- | --- |
| **fract hash instability** | read blog - [**Hash Noise stability in GPU Shaders**](https://arugl.medium.com/hash-noise-in-gpu-shaders-210188ac3a3e) |
| <img src="https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/Xc23DW_1.jpg" width="650" height="auto" /> | [BUG TEST Rough Seas fract hash](https://www.shadertoy.com/view/Xc23DW) - Just test shader for this blog post [**Hash Noise stability in GPU Shaders**](https://arugl.medium.com/hash-noise-in-gpu-shaders-210188ac3a3e), [blog mirror](https://danilw.github.io/blog/Hash_Noise_in_GPU_Shaders/). |
| <img src="https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/4fSXDd.png" width="650" height="auto" /> | Related-example of fract-hash instability described in blog post [**Hash Noise stability in GPU Shaders**](https://arugl.medium.com/hash-noise-in-gpu-shaders-210188ac3a3e), [blog mirror](https://danilw.github.io/blog/Hash_Noise_in_GPU_Shaders/). [Shader link](https://www.shadertoy.com/view/lcBSWd). |
| <img src="https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/tX2GRm.png" width="650" height="auto" /> | One more real-life example of fract-hash instability [Shader link](https://www.shadertoy.com/view/tX2GRm). |
| --- | --- |
| <img src="https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/NtfyD2.png" width="650" height="auto" /> | [BUG Nvidia OpenGL ruins float](https://www.shadertoy.com/view/NtfyD2) - Nvidia OpenGL GLSL compiler somehow ruins float, look code. |
| <img src="https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/WscyRM.png" width="650" height="auto" /> | [Test shader - OS detector](https://www.shadertoy.com/view/WscyRM) - Nvidia 4060 - framebuffers and youtube-panorama videos are broken in Chrome-Vulkan. [Video](https://youtu.be/gTKirt_50sg), [*Chrome*](https://issues.chromium.org/issues/326987168). |
| <img src="https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/l3fXzn.png" width="650" height="auto" /> | [BUG Float loop iterations](https://www.shadertoy.com/view/l3fXzn) - Never use float as loop-iterator, this can be considered as "not a bug" and expected, but Nvidia have some special float-behavior in OpenGL, I think this is reason for this. |
| <img src="https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/MX2Xzh.png" width="650" height="auto" /> | [BUG AMD textureGrad vs discard](https://www.shadertoy.com/view/MX2Xzh) - Bug - it look like conflict of discard and textureGrad, bug works only in Linux on new mesa in [*AMD*](https://gitlab.freedesktop.org/mesa/mesa/-/issues/10807) in OpenGL, *found  it in [this](https://www.shadertoy.com/view/4fsSRn) shader*. |
| <img src="https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/MfsBz8.jpg" width="650" height="auto" /> | [BUG AMD performance dFd if ruin](https://www.shadertoy.com/view/MfsBz8) - When this specific shader run in 32-bit buffer - it get 1000x slowdown because use of fwidth/dFd and if condition after - idk it so weird, when it run in 8-bit buffer - its fine no slowdown, related to shader compiler I think. [AMD](https://gitlab.freedesktop.org/mesa/mesa/-/issues/11683) |

[BUG Nvidia Vulkan driver no comp](https://www.shadertoy.com/view/XcB3Wm) - [Nvidia](https://forums.developer.nvidia.com/t/nvidia-vulkan-driver-use-6-2gb-ram-and-crash-on-compiling-simple-shader-driver-bug/277851) Vulkan driver compiler bug, happening in [*Chrome*](https://bugs.chromium.org/p/chromium/issues/detail?id=1515387)

[BUG Angle 20x slowdown](https://www.shadertoy.com/view/flSSDV) - DX11 [FXC bug](https://bugs.chromium.org/p/chromium/issues/detail?id=1238461)

[BUG AMD branching optimization](https://www.shadertoy.com/view/4f23D3) - I do not know is it AMD or ANGLE bug - bug works only in ANGLE, and I tested it only in AMD.

[BUG Chrome or Nvidia compiling](https://www.shadertoy.com/view/fsSSzd) - Nvidia OpenGL compiler bug

[BUG Vulkan AMD crash](https://www.shadertoy.com/view/wdfcDX) - old bug, [fixed already](https://community.amd.com/thread/250887)

[BUG Vulkan AMD loop bug](https://www.shadertoy.com/view/tsXyDH) - old bug, [fixed already](https://community.amd.com/message/2964120)

[BUG Vulkan Nvidia noise bug](https://www.shadertoy.com/view/ttjcRW) - [fixed already](https://forums.developer.nvidia.com/t/vulkan-shader-bug-can-someone-confirm-is-this-only-my-bug/140392), but very weird - basically all shaders that use fract and mod was bugged in Nvidia Vulkan for 5 years and no one noticed... Videos of this bug [1](https://youtu.be/hBbI2rw18ew) [2](https://youtu.be/KEqHarMBmOU)

[BUG OpenGL Nvidia high GPU usage](https://www.shadertoy.com/view/tdfGWS) - Nvidia OpenGL compiler bug

[BUG smoothstep( 1, 0 ,0) specs](https://www.shadertoy.com/view/tdf3zf) - `smoothsep` on GPU does not follow specs, when on CPU it does.

[BUG Nvidia OpenGL compiler bug](https://www.shadertoy.com/view/NsdXRs) - Nvidia OpenGL compiler bug, fixed in 510+ driver.

[Old GLES2 depth write bug on AMD](https://community.amd.com/t5/opengl-vulkan/amd-gles2-depth-write-broken/m-p/279353) - still broken, I think it another wontfix because no one care about OpenGL. (*Note* - by clicking this link you may trigger interesting web-bug, look below "*Interesting amd forums bug*" - just press enter in address text in webbrowser if you get 403)
___

## *Not a Bug*, its expected behavior but it still weird

| Screenshot, click to open | Bug link |
|-------------|------------|
| <img src="https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/4fffR7_ip.jpg" width="650" height="auto" /> | [one of my shaders on iPhone - video webm link](https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/4fffR7_ip.webm) This my shader [TV Scene, wall of TV](https://www.shadertoy.com/view/4fffR7) run on iPhone - fad from Shadertoy discord shared this video. |
| <img src="https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/sllXW8.png" width="650" height="auto" /> | [BUG 32-bit float precision](https://www.shadertoy.com/view/sllXW8) - related to blog post [Float precision on GPU, bugs/features](https://arugl.medium.com/float-precision-on-gpu-bugs-features-178ddd030f) |
| <img src="https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/ftXXW4.png" width="650" height="auto" /> | [BUG 32-bit float precision test](https://www.shadertoy.com/view/ftXXW4) - Look function `colorAniso()` in shader, for some reason this noise happens when camera look on plane(explained [there](https://www.shadertoy.com/view/ftlcDS)). Also I use this shader to compare 32bit floats vs 64 bit floats. (more shaders to compare float vs double [Shadertoy shader, list in comments](https://www.shadertoy.com/view/7sdyz2) ) |
| <img src="https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/NlXXWS.png" width="650" height="auto" /> | [BUG GPU driver unroll prediction](https://www.shadertoy.com/view/NlXXWS) - using break in loop adds precision to operation, expected because shader compiler compile shader diferently for each case. |
| <img src="https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/ftXSWB.png" width="650" height="auto" /> | [BUG GPU precision never 0](https://www.shadertoy.com/view/ftXSWB) - related to blog post [Float precision on GPU, bugs/features](https://arugl.medium.com/float-precision-on-gpu-bugs-features-178ddd030f) |
| <img src="https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/fsc3RM.png" width="650" height="auto" /> | [BUG Nvidia Vulkan UB behavior](https://www.shadertoy.com/view/fsc3RM) - weird UB behavior that result not same image depends on GPU, Vulkan only |
| <img src="https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/nv_double_specs_crash.png" width="650" height="auto" /> | Using double in functions that does not support double may result crash after launching shader, on screenshot application crashed after rendering first frame in shader code `exp(dvec2(sqrt`(*exp* can only precess 16 and 32 bit floats [by specs](https://www.khronos.org/registry/SPIR-V/specs/unified1/GLSL.std.450.html)). Link to zip with exe to test [bug_nv_double_specs_crash.zip](https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/bug_nv_double_specs_crash.zip) |
| <img src="https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/7lt3Rl.png" width="650" height="auto" /> | [BUG normalize bugs](https://www.shadertoy.com/view/7lt3Rl) - related to [Cubemap GLSL shader debug functions](https://arugl.medium.com/cubemap-glsl-shader-debug-functions-3f4c659e7833) - normalize does not return vector with length 1.0 |
| <img src="https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/NsBBDW.png" width="650" height="auto" /> | [BUG float precision patterns](https://www.shadertoy.com/view/NsBBDW) - patterns of sin, sqrt diferent between GPUs (look screenshot), related to [Float precision on GPU, bugs/features](https://medium.com/@arugl/float-precision-on-gpu-bugs-features-178ddd030f) |
| <img src="https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/ftXcW7.png" width="650" height="auto" /> | [BUG linear interpolation test](https://www.shadertoy.com/view/ftXcW7) - float linear interpolation test on diferent GPU diferent result (look comments in code) |
| <img src="https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/ftlcDS_2.png" width="650" height="auto" /> | [BUG 32-bit float pattern 2](https://www.shadertoy.com/view/ftlcDS) - behavior related to losing float precision on small and large numbers, _read comment and look two screenshots on shadertoy page_. When screen has square size then determinant and inverse return some small and large numbers that leads to precision loss and this pattern on screenshot. |
| <img src="https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/upl_demos/pathtracer/repetition_pattern.png" width="650" height="auto" /> | [TEST noise repetition](https://www.shadertoy.com/view/NdGfRd) - patters of fract and sin hash and noise, by searching same values on image. [histogram of noise](https://www.shadertoy.com/view/7syfRd) |
| <img src="https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/cdB3WG.png" width="650" height="auto" /> | [BUG STL slowdown](https://www.shadertoy.com/view/cdB3WG) - Only Nvidia OpenGL slowdown, OpenGL Nvidia shader compiler for some reason dont like when arrray is argument. Related to [Decompiling Nvidia shaders, and optimizing](https://arugl.medium.com/decompiling-nvidia-shaders-and-optimizing-5aeaeb65f828) blog.|
| <img src="https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/dd23Dc.png" width="650" height="auto" /> | [Optimized ML/Neural shader](https://www.shadertoy.com/view/dd23Dc) - Nvidia case when CONST buffer become too large and it destroy performance. Related to [Decompiling Nvidia shaders, and optimizing](https://arugl.medium.com/decompiling-nvidia-shaders-and-optimizing-5aeaeb65f828) blog.|
| <img src="https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/mdfSz8.png" width="650" height="auto" /> | [BUG Nvidia float as argument bug](https://www.shadertoy.com/view/mdfSz8) - (OpenGL and Vulkan) Nvidia OpenGL GLSL compiler somehow generates wrong floats when const floats used as argument of function, look code. (look like this is not bug, its compiler inconsistency, read shader code comments) (the "bug" is - `floor((900./37.)*1000.)/1000.` is 24.323999 on CPU and 24.324001 on GPU) |
| <img src="https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/dtXGW4_vk_nv.png" width="650" height="auto" /> | **TEST - About fract-hash consistensy**. Comparison for [Rough Seas shader](https://www.shadertoy.com/view/dtXGW4), discussion [Hash without Sine](https://www.shadertoy.com/view/4djSRW). **Look this two gif** - [**OpenGL AMD *(red)* and Nvidia _(green)_**](https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/dtXGW4_gl_amd_nv.gif) and [**Vulklan AMD when *iTime* calculated on GPU as *non-const* and when it is _const_**](https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/dtXGW4_vk_amd_iTime.gif). On screenshot left - Vulkan Nvidia there appears lines I think because of *sincos* implementation on Nvidia. To test - `#define iTime (8998.3+floor((900./37.)*1000.+float(min(iFrame,0)))/1000.)` to *Rough Seas shader* code, to test *iTime as const* - remove `+float(min(iFrame,0))`.|
| <img src="https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/mt33RH_2.png" width="650" height="auto" /> | [BUG Cubemap rendering order on load](https://www.shadertoy.com/view/mt33RH) - WebGL behavior depends on GPU and rendering API. Watch [this youtube video](https://youtu.be/CFCvSRuSB6s). |
| <img src="https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/Dl3Szn_1.png" width="650" height="auto" /> | [BUG float calc inconsistency](https://www.shadertoy.com/view/Dl3Szn) - Bug is - I was debugging/making TAA and was comparing result of "my controlable camera" to "static camera" - and result were not equal when camera matrix and everything was equal, I tested every single value line by line, and *in TAA I was worrying that I miss "pixel jittering" somewhere* and result was pointing on it... to not spread confusstion - this is "not bug" I just forgot that GPU are inconsistant in float computations, and when two equal values come from different sources result of operations on them will be not equal to same float value that come from single source. |
| <img src="https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/dldXR4.png" width="650" height="auto" /> | [BUG float calc inconsistency](https://www.shadertoy.com/view/dldXR4) - Bug is - result of `ro1 + rd - ro2` where ro1 and ro2 is equal - will not be equal to rd. Same as [BUG GPU precision never 0](https://www.shadertoy.com/view/ftXSWB). |
| <img src="https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/bug_nv_s_ub.png" width="650" height="auto" /> | Bug with Undefined Behavior in Nvidia GPUs, and since this is UB it can be considered as expected behavior. This is third version of similar UB behaviour bug I reported before(to Nvidia and [*Chrome*](https://bugs.chromium.org/p/chromium/issues/detail?id=1425685) in bugreports). Most strange for me(and why I think this is bug) - visual result generated by shader changed/reacting to outside activity (other apps that use GPU). **Watch [this video(Youtube)](https://youtu.be/tpG84rvTsJI) to see this bug behavior.** Download exe/linux version [bug_nv_ub_leaks_v3_1.zip](https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/bug_nv_ub_leaks_v3_1.zip) with shader source. |
| <img src="https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/XcjyWz.jpg" width="650" height="auto" /> | [4-th edition of bug above](https://www.shadertoy.com/view/XcjyWz) with uninitialized variables, but now it work on AMD GPU(only OpenGL only Linux), when before it was not. This [youtube video](https://youtu.be/sZFNDh3JzfI) show it working. Mentioned in [this changelog](https://www.mozilla.org/en-US/security/advisories/mfsa2024-33/) *(search s48)*. |
| <img src="https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/sltGWj.jpg" width="650" height="auto" /> | [BUG Mip inconsistent Vulkan/Ogl](https://www.shadertoy.com/view/sltGWj) - When **sampler2D have *no mipmaps* but *has linear filter* and shader *has push_const*** - functions `texture` has some unpredictable result. (woks only in this specific shader(shader made from other large shader where I found this bug)). Bug does not work when shader does not have push_constants. Bug work on Shadertoy only in OpenGL mode `chrome.exe --use-angle=gl` and only when FBO has mipmaps. But in OpenGL this is "not a bug" or atleast it can be explained. **Update 2023** - now this bug works in Webbgowser but *mipmaps required* [*Chrome*](https://bugs.chromium.org/p/chromium/issues/detail?id=1497211). [**Youtube video of this bug**](https://youtu.be/W3jd-zEZTek). **This shader also include second bug/behavior** - on video 00:31 - but I think this is Shadertoy website behavior - alpha-value of mipmap change not same as rgb value when it supposed to be same. |
| <img src="https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/4cdSR4.png" width="650" height="auto" /> | [BUG WebGL wrong compile error](https://www.shadertoy.com/view/4cdSR4) - `1./0.` generate uintBitsToFloat error in *Firefox* - for some reason. |
| <img src="https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/ff310.png" width="650" height="auto" /> | [Firefox bugreport with details and code](https://bugzilla.mozilla.org/show_bug.cgi?id=1922497) - `float [1024] arr;` - Firefox 130 does not allow const data `>=1024` - incorrectly ignoring MAX_FRAGMENT_UNIFORM_COMPONENTS |
| <img src="https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/McGGz1.png" width="650" height="auto" /> | Chrome 129 very weird bug - [watch this video-clip](https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/McGGz1_ins_chrome.mp4) - in Shadertoy buffer only - variable can not be named with `outColor` - it works in Firefox. |
| <img src="https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/4cyXRh_vs_XcyXRh.png" width="650" height="auto" /> | [TEST bits 32 r/w block[8x2]](https://www.shadertoy.com/view/XcyXRh) and [TEST bits r/w block[8x8x8]](https://www.shadertoy.com/view/4cyXRh) - Again about STL in Nvidia shader, read blog [Decompiling Nvidia shaders](https://medium.com/geekculture/decompiling-nvidia-shaders-and-optimizing-5aeaeb65f828) or [*blog-mirror*](https://danilw.github.io/blog/decompiling_and_optimizing_nvidia_shaders/). Result OpenGL Nvidia 4060 block[8x8x8] - 20FPS on 1080p, block[8x2] - 60fps on 1080p. Vulkan - I get 60fps on both but with block[8x8x8] is 100% GPU usage when block[8x2] is 30% GPU usage. |
| <img src="https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/4cKfzc.png" width="650" height="auto" /> | mat3x2(mat2x3) [GLSL convertion order](https://www.shadertoy.com/view/4cKfzc) |

[Nvidia OpenGL can not compile this shader](https://www.shadertoy.com/view/4tKczz) - arrays - for some reason Nvidia driver(555) compiler crash because of mi_o_y function - I fixed it by clamp array range, if remove clamp in arrays in mi_o_y function - bug will work.

[BUG Vulkan uint32 128 bytes push constant](https://www.shadertoy.com/view/3sXyW2) (does not work from WebGL, need modification read Shadertoy comment) - (not a bug) - just reminder for myself: in Vulkan - *Any member of a push constant block that is declared as an array must only be accessed with dynamically uniform indices*. If you use array as push const then result will be this - [screenshot 1](https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/vk_amd_128pc.png), [screenshot 2](https://danilw.github.io/GLSL-howto/test_AMD_shader_ub/amd.png). Do not use array as push-constant in Vulkan. [khronos.org/registry/vulkan/specs](https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/chap15.html#interfaces-resources-pushconst)

[BUG Vulkan not equal to OpenGL](https://www.shadertoy.com/view/slsXzs) - UB behavior in OpenGL and Vulkan. I think it because CPU-side precompilation result UB diferently.

[BUG cubemap rayDir to fragCoord](https://www.shadertoy.com/view/7l33W2) - related to [Cubemap GLSL shader debug functions](https://arugl.medium.com/cubemap-glsl-shader-debug-functions-3f4c659e7833)

[BUG tile rendering dFd test](https://www.shadertoy.com/view/NsXBWr) - not a bug, its expected behavior - dFd and texture functions return wrong result on edges when tile size not even. Related to [tile rendering example](https://www.shadertoy.com/view/tltBzM) and [advanced tile render](https://www.shadertoy.com/view/7ldXzf) (look line 4 in Common)

[AMD OpenGL bindless seems bugged](https://youtu.be/Bug2vGm9HZI) - links to bugreports [1](https://gitlab.freedesktop.org/mesa/mesa/-/issues/10581) [2](https://community.amd.com/t5/newcomers-start-here/opengl-4-1-this-api-capture-crash-amd-gpu-driver-on-window-linux/m-p/664656/highlight/true#M2951) [3](https://github.com/DiligentGraphics/DiligentSamples/issues/140). I was debugging/looking `GL_ARB_bindless_texture` in app [Nu.Gaia](https://github.com/bryanedds/Nu) originally and coult not see obvious reasons why it not work on AMD only. <br>[Khronos Bindless_Texture](https://www.khronos.org/opengl/wiki/Bindless_Texture) say - *"Bindless textures are not safe. The API is given fewer opportunities to ensure sane behavior; it is up to the programmer to maintain integrity."* - and there no tools to debug, renderdoc does not support bindless, only what working I found is [apitrace](https://github.com/apitrace/apitrace) and [glintercept](https://github.com/dtrebilco/glintercept/releases/tag/1.3.4) - but they also show everything same on amd/nvidia, so I have no idea why this is not working on amd. As you see on video - other apps that use bindless also bugged on amd(only Vulkan bindless work on AMD), so I assumed, may be wrong - that "it is amd driver bug/behavior with bindless".

[Chrome Vulkan: Fix line-loop draw arrays after elements](https://issues.chromium.org/issues/40911000) - Google confirmed and fixed one Vulkan related bug in their browser.

*[Interesting amd forums bug](https://youtu.be/B7v4yk8-wzY), not related to graphics, just funny. (read description there)* 

___

*Angle experience* - some of bug-related experience (unexpected behavior when your code is fine) - launching my [GLSL card game in ANGLE fist time (video)](https://youtu.be/uY15AZfesU4), Godot in ANGLE [FBO size jump (video)](https://youtu.be/rqmQ7EnDmb8), till 2019 (pre 76 version) Chrome had [broken instanced particles support](https://github.com/godotengine/godot/issues/28573).
___

### Driver updates:

Summer 2021 AMD update their driver (Linux and Windows, close and opensource) with fixing all my bugs that I reported during 2020-2021. Vulkan only fixes, OpenGL still very broken there. (that makes my [GLSL Auto Tetris](https://medium.com/geekculture/launching-619-thousand-tetris-on-gpu-their-rendering-and-a-simple-bot-f2449b607db1) and other my demos work in Vulkan on AMD GPUs)

In 2023-2024 Nvidia and AMD also fixed some Vulkan bugs from this list - but most of OpenGL bugs is "wontfix" at this point. 
*(and some security bugs that I not listed here also were fixed, on both gpus)*
