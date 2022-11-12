
# GPU-my-list-of-bugs

**what is it** - list of bugs I found writing shaders, mostly shader bugs.

### Contact: [**Join discord server**](https://discord.gg/JKyqWgt)
___

## Debug:

### To **test this bugs in Vulkan**:

*(some shaders require modification of Vulkan launcher, or works only in OpenGL, read description on each bug-shader)*

Download *empty_template_shadertoy.zip* from **[vulkan-shadertoy-launcher Release](https://github.com/danilw/vulkan-shadertoy-launcher/releases)** and copy-paste Shadertoy shader to `shaders/shadertoy/*.glsl` files.

### To **test in OpenGL**:

Use Linux or in Windows just launch Webbroser with disabled Angle `chrome.exe --use-angle=gl`

### To test shaders result in CPU-shader emulation:

*Use swiftshader* (works on every platform but this is ***not best option***) 

`google-chrome --incognito --use-gl=swiftshader-webgl`

Many of listed shaders *will crash swiftshader* or have completely wrong result because bugs in swiftshader.

**Use OpenGL Mesa LLVM (llvmpipe) OpenGL emulation** (***best option***) 

`Xephyr -br -ac -noreset -screen 1280x720 :10&`

And launch firefox there `DISPLAY=:10 XDG_SESSION_TYPE=x11 firefox`

It will render WebGL on CPU llvmpipe driver. (do not launch chrome this way, chrome will use swiftshader instead of llvmpipe). To see that *llvmpipe* used - `DISPLAY=:10 glxinfo | grep OpenGL | grep string`

Other way is try to use - `LIBGL_ALWAYS_SOFTWARE=1 __GLX_VENDOR_LIBRARY_NAME=mesa VK_ICD_FILENAMES=/usr/share/vulkan/icd.d/lvp_icd.i686.json:/usr/share/vulkan/icd.d/lvp_icd.x86_64.json <command>`

**Use Vulkan Mesa LLVM lavapipe (llvmpipe) Vulkan emulation** (*can be unstable*) 

In Linux install package something like `libvulkan_lvp - Mesa vulkan driver for LVP` (use package search in your Linux).

And select `--gpu=0` in my vulkan-shadertoy-launcher launch line option. GPU should be detected as:

`Vulkan GPU - CPU: llvmpipe (LLVM 13.0.0, 256 bits) (id: 0x0000) from vendor 0x10005 [driver version: 0x0001, API version: 0x4020C3]`

___

## List of shader bugs:

### Shadertoy links, click on Screenshot previews to see full image.

| Screenshot, click to open | Bug link |
|-------------|------------|
| <img src="https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/wdVyWD.png" width="650" height="auto" /> | [BUG ANGLE dFd broken on break](https://www.shadertoy.com/view/wdVyWD) - Angle bug |
| <img src="https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/NslGR4.png" width="650" height="auto" /> | [BUG Nvidia const to array](https://www.shadertoy.com/view/NslGR4) (array index bug) - Nvidia OpenGL and Vulkan driver shader compiler bug, [Other Nvidia OpenGL bugs in array](https://forums.developer.nvidia.com/t/opengl3-out-in-mat4-broken-on-many-nvidia-videocards-in-vertex-shader-shader-code-included/145921)  |
| <img src="https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/NtlyWn.png" width="650" height="auto" /> | [BUG Vulkan AMD array indexing](https://www.shadertoy.com/view/NtlyWn) (array index bug) - AMD Vulkan driver bug, [bugreport](https://community.amd.com/t5/opengl-vulkan/vulkan-amd-arrays-or-precompiling-spirv-bug/m-p/517283#M4172). And **if you combine this code with Nvidia Vulkan array indexing bug - you can have shader that bugged on both Nvidia and AMD at same time** (yeee super cool). |
| <img src="https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/wdXGW8.png" width="650" height="auto" /> | [BUG Rand/Hash pre-calculated](https://www.shadertoy.com/view/wdXGW8) - I think this is not expected, related to blog post [Float precision on GPU, bugs/features](https://arugl.medium.com/float-precision-on-gpu-bugs-features-178ddd030f) |
| <img src="https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/tlfBRB.png" width="650" height="auto" /> | [BUG floatBitsToUint comp vs real](https://www.shadertoy.com/view/tlfBRB) - related to blog post [Float precision on GPU, bugs/features](https://arugl.medium.com/float-precision-on-gpu-bugs-features-178ddd030f) |
| <img src="https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/stK3WG.png" width="650" height="auto" /> | [BUG Vulkan Nvidia sin hash](https://www.shadertoy.com/view/stK3WG) - sin hash broken in Vulkan on Nvidia, when in OpenGL everything fine. |
| <img src="https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/7ddfzr.jpg" width="650" height="auto" /> | [BUG sin not same on AMD/Nvidia](https://www.shadertoy.com/view/7ddfzr) - similar as "sin hash broken" above, but in not-noise shader, in fractal shader sin making different visual image. |
| <img src="https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/NtfyD2.png" width="650" height="auto" /> | [BUG Nvidia OpenGL ruins float](https://www.shadertoy.com/view/NtfyD2) - Nvidia OpenGL GLSL compiler somehow ruins float, look code. |

[BUG Angle 20x slowdown](https://www.shadertoy.com/view/flSSDV) - DX11 [FXC bug](https://bugs.chromium.org/p/chromium/issues/detail?id=1238461)

[BUG Chrome or Nvidia compiling](https://www.shadertoy.com/view/fsSSzd) - Nvidia OpenGL compiler bug

[BUG Vulkan AMD crash](https://www.shadertoy.com/view/wdfcDX) - old bug, [fixed already](https://community.amd.com/thread/250887)

[BUG Vulkan AMD loop bug](https://www.shadertoy.com/view/tsXyDH) - old bug, [fixed already](https://community.amd.com/message/2964120)

[BUG Vulkan Nvidia noise bug](https://www.shadertoy.com/view/ttjcRW) - [fixed already](https://forums.developer.nvidia.com/t/vulkan-shader-bug-can-someone-confirm-is-this-only-my-bug/140392), but very weird - basically all shaders that use fract and mod was bugged in Nvidia Vulkan for 5 years and no one noticed... Videos of this bug [1](https://youtu.be/hBbI2rw18ew) [2](https://youtu.be/KEqHarMBmOU)

[BUG OpenGL Nvidia high GPU usage](https://www.shadertoy.com/view/tdfGWS) - Nvidia OpenGL compiler bug

[BUG smoothstep( 1, 0 ,0) specs](https://www.shadertoy.com/view/tdf3zf) - `smoothsep` on GPU does not follow specs, when on CPU it does.

[BUG Nvidia OpenGL compiler bug](https://www.shadertoy.com/view/NsdXRs) - Nvidia OpenGL compiler bug, fixed in 510+ driver.
___

## *Not a Bug*, its expected behavior but it still weird

| Screenshot, click to open | Bug link |
|-------------|------------|
| <img src="https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/7tjGRW.png" width="650" height="auto" /> | [BUG Nvidia OpenGL arrays wrong](https://www.shadertoy.com/view/7tjGRW) (array and matX indexing) - only Nvidia OpenGL. Can be considered as "not a bug" because UB line 29 `ext_3[return_zero()] += -2147483648` as function argument, so in unknown what will be first `+=` or passing as parameter. |
| <img src="https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/sllXW8.png" width="650" height="auto" /> | [BUG 32-bit float precision](https://www.shadertoy.com/view/sllXW8) - related to blog post [Float precision on GPU, bugs/features](https://arugl.medium.com/float-precision-on-gpu-bugs-features-178ddd030f) |
| <img src="https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/ftXXW4.png" width="650" height="auto" /> | [BUG 32-bit float precision test](https://www.shadertoy.com/view/ftXXW4) - Look function `colorAniso()` in shader, for some reason this noise happens when camera look on plane(explained [there](https://www.shadertoy.com/view/ftlcDS)). Also I use this shader to compare 32bit floats vs 64 bit floats. (more shaders to compare float vs double [Shadertoy shader, list in comments](https://www.shadertoy.com/view/7sdyz2) ) |
| <img src="https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/NlXXWS.png" width="650" height="auto" /> | [BUG GPU driver unroll prediction](https://www.shadertoy.com/view/NlXXWS) - using break in loop adds precision to operation, expected because shader compiler compile shader diferently for each case. |
| <img src="https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/ftXSWB.png" width="650" height="auto" /> | [BUG GPU precision never 0](https://www.shadertoy.com/view/ftXSWB) - related to blog post [Float precision on GPU, bugs/features](https://arugl.medium.com/float-precision-on-gpu-bugs-features-178ddd030f) |
| <img src="https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/fsc3RM.png" width="650" height="auto" /> | [BUG Nvidia Vulkan UB behavior](https://www.shadertoy.com/view/fsc3RM) - weird UB behavior that result not same image depends on GPU, Vulkan only |
| <img src="https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/nv_double_specs_crash.png" width="650" height="auto" /> | Using double in functions that does not support double may result crash after launching shader, on screenshot application crashed after rendering first frame in shader code `exp(dvec2(sqrt`(*exp* can only precess 16 and 32 bit floats [by specs](https://www.khronos.org/registry/SPIR-V/specs/unified1/GLSL.std.450.html)). Link to zip with exe to test [bug_nv_double_specs_crash.zip](https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/bug_nv_double_specs_crash.zip) |
| <img src="https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/bug_nv_s_ub.png" width="650" height="auto" /> | Bug with Undefined Behavior in Nvidia GPUs, and since this is UB it can be considered as expected behavior. This is third version of similar UB behaviour bug I reported before(to Nvidia in bugreports). Most strange for me(and why I think this is bug) - visual result generated by shader changed/reacting to outside activity (other apps that use GPU). **Watch [this video(Youtube)](https://youtu.be/tpG84rvTsJI) to see this bug behavior.** Or look this [description on reddit](https://www.reddit.com/r/vulkan/comments/toa0pd/can_this_ub_behavior_in_glslspirv_shader_can_be/). Download exe/linux version [bug_nv_ub_leaks_v3_1.zip](https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/bug_nv_ub_leaks_v3_1.zip) with shader source. |
| <img src="https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/7lt3Rl.png" width="650" height="auto" /> | [BUG normalize bugs](https://www.shadertoy.com/view/7lt3Rl) - related to [Cubemap GLSL shader debug functions](https://arugl.medium.com/cubemap-glsl-shader-debug-functions-3f4c659e7833) - normalize does not return vector with length 1.0 |
| <img src="https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/NsBBDW.png" width="650" height="auto" /> | [BUG float precision patterns](https://www.shadertoy.com/view/NsBBDW) - patterns of sin, sqrt diferent between GPUs (look screenshot), related to [Float precision on GPU, bugs/features](https://medium.com/@arugl/float-precision-on-gpu-bugs-features-178ddd030f) |
| <img src="https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/ftXcW7.png" width="650" height="auto" /> | [BUG linear interpolation test](https://www.shadertoy.com/view/ftXcW7) - float linear interpolation test on diferent GPU diferent result (look comments in code) |
| <img src="https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/ftlcDS_2.png" width="650" height="auto" /> | [BUG 32-bit float pattern 2](https://www.shadertoy.com/view/ftlcDS) - behavior related to losing float precision on small and large numbers, _read comment and look two screenshots on shadertoy page_. When screen has square size then determinant and inverse return some small and large numbers that leads to precision loss and this pattern on screenshot. |
| <img src="https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/upl_demos/pathtracer/repetition_pattern.png" width="650" height="auto" /> | [TEST noise repetition](https://www.shadertoy.com/view/NdGfRd) - patters of fract and sin hash and noise, by searching same values on image. [histogram of noise](https://www.shadertoy.com/view/7syfRd) |
| <img src="https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/cdB3WG.png" width="650" height="auto" /> | [BUG STL slowdown](https://www.shadertoy.com/view/cdB3WG) - Only Nvidia OpenGL slowdown, OpenGL Nvidia shader compiler for some reason dont like when arrray is argument. Related to [Decompiling Nvidia shaders, and optimizing](https://arugl.medium.com/decompiling-nvidia-shaders-and-optimizing-5aeaeb65f828) blog.|
| <img src="https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/images_bugs/dd23Dc.png" width="650" height="auto" /> | [Optimized ML/Neural shader](https://www.shadertoy.com/view/dd23Dc) - Nvidia case when CONST buffer become too large and it destroy performance. Related to [Decompiling Nvidia shaders, and optimizing](https://arugl.medium.com/decompiling-nvidia-shaders-and-optimizing-5aeaeb65f828) blog.|

[BUG Vulkan uint32 128 bytes push constant](https://www.shadertoy.com/view/3sXyW2) (does not work from WebGL, need modification read Shadertoy comment) - (not a bug) - just reminder for myself: in Vulkan - *Any member of a push constant block that is declared as an array must only be accessed with dynamically uniform indices*. If you use array as push const then result will be this - [screenshot 1](https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/vk_amd_128pc.png), [screenshot 2](https://danilw.github.io/GLSL-howto/test_AMD_shader_ub/amd.png). Do not use array as push-constant in Vulkan. [khronos.org/registry/vulkan/specs](https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/chap15.html#interfaces-resources-pushconst)

[BUG Mip inconsistent Vulkan/Ogl](https://www.shadertoy.com/view/sltGWj) - When **sampler2D have *no mipmaps* but *has linear filter* and shader *has push_const*** - functions `texture` has some unpredictable result. (woks only in this specific shader(shader made from other large shader where I found this bug)). Bug does not work when shader does not have push_constants. Bug work on Shadertoy only in OpenGL mode `chrome.exe --use-angle=gl` and only when FBO has mipmaps. But in OpenGL this is "not a bug" or atleast it can be explained. [youtube video of this bug](https://youtu.be/OcpBVHMb88M)

[BUG Vulkan not equal to OpenGL](https://www.shadertoy.com/view/slsXzs) - UB behavior in OpenGL and Vulkan. I think it because CPU-side precompilation result UB diferently.

[BUG cubemap rayDir to fragCoord](https://www.shadertoy.com/view/7l33W2) - related to [Cubemap GLSL shader debug functions](https://arugl.medium.com/cubemap-glsl-shader-debug-functions-3f4c659e7833)

[BUG tile rendering dFd test](https://www.shadertoy.com/view/NsXBWr) - not a bug, its expected behavior - dFd and texture functions return wrong result on edges when tile size not even. Related to [tile rendering example](https://www.shadertoy.com/view/tltBzM) and [advanced tile render](https://www.shadertoy.com/view/7ldXzf) (look line 4 in Common)

___

*Angle experience* - some of bug-related experience (unexpected behavior when your code is fine) - launching my [GLSL card game in ANGLE fist time (video)](https://youtu.be/uY15AZfesU4), Godot in ANGLE [FBO size jump (video)](https://youtu.be/rqmQ7EnDmb8), till 2019 (pre 76 version) Chrome had [broken instanced particles support](https://github.com/godotengine/godot/issues/28573).
___

### Driver updates:

Summer 2021 AMD update their driver (Linux and Windows, close and opensource) with fixing all my bugs that I reported during 2020-2021. Vulkan only fixes, OpenGL still very broken there. (that makes my [GLSL Auto Tetris](https://medium.com/geekculture/launching-619-thousand-tetris-on-gpu-their-rendering-and-a-simple-bot-f2449b607db1) and other my demos work in Vulkan on AMD GPUs)

Nvidia 510+ version (Linux and Windows, begin of 2022 update) - their SPIR-V compiler in driver fix some bugs with mat/vec/array index and some memory-[leaks (video)](https://youtu.be/puPplT1nPCI) from shaders also fixed, some bugs I listed also stop working in Vulkan(but still works in OpenGL), some bugs changed behavior.

