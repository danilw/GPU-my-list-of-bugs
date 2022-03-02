
# GPU-my-list-of-bugs

**what is it** - list of bugs I found writing shaders, mostly shader bugs.

### Contact: [**Join discord server**](https://discord.gg/JKyqWgt)
___

## Debug:

### To **test this bugs in Vulkan**:

*(some shaders require modification of Vulkan launcher, or works only in OpenGL)*

Use **[vulkan-shadertoy-launcher Release](https://github.com/danilw/vulkan-shadertoy-launcher/releases)** and copy-paste Shadertoy shader to `shaders/shadertoy/*.glsl` files.

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

**Use Vulkan Mesa LLVM lavapipe (llvmpipe) Vulkan emulation** (*can be unstable*) 

In Linux install package something like `libvulkan_lvp - Mesa vulkan driver for LVP` (use package search in your Linux).

And select `--gpu=0` in my vulkan-shadertoy-launcher launch line option. GPu should be detected as:

`Vulkan GPU - CPU: llvmpipe (LLVM 13.0.0, 256 bits) (id: 0x0000) from vendor 0x10005 [driver version: 0x0001, API version: 0x4020C3]`

___

## List of shader bugs (Shadertoy links, look *in comments* on Shadertoy page for *screenshot of bug*)

[BUG ANGLE dFd broken on break](https://www.shadertoy.com/view/wdVyWD) - Angle bug

[BUG Angle 20x slowdown](https://www.shadertoy.com/view/flSSDV) - DX11 [FXC bug](https://bugs.chromium.org/p/chromium/issues/detail?id=1238461)

[BUG Chrome or Nvidia compiling](https://www.shadertoy.com/view/fsSSzd) - Nvidia OpenGL compiler bug

[BUG Nvidia OpenGL compiler bug](https://www.shadertoy.com/view/NsdXRs) - Nvidia OpenGL compiler bug

[BUG Nvidia OpenGL arrays wrong](https://www.shadertoy.com/view/7tjGRW) (array and matX indexing) - only Nvidia OpenGL, [Other Nvidia OpenGL bugs](https://forums.developer.nvidia.com/t/opengl3-out-in-mat4-broken-on-many-nvidia-videocards-in-vertex-shader-shader-code-included/145921) (look there for more matX bugs)

[BUG Nvidia const to array](https://www.shadertoy.com/view/NslGR4) (array index bug) - Nvidia OpenGL and Vulkan driver shader compiler bug

[BUG Rand/Hash pre-calculated](https://www.shadertoy.com/view/wdXGW8) - I think this is not expected, related to blog post [Float precision on GPU, bugs/features](https://arugl.medium.com/float-precision-on-gpu-bugs-features-178ddd030f)

[BUG Vulkan AMD crash](https://www.shadertoy.com/view/wdfcDX) - old bug, [fixed already](https://community.amd.com/thread/250887)

[BUG Vulkan AMD loop bug](https://www.shadertoy.com/view/tsXyDH) - old bug, [fixed already](https://community.amd.com/message/2964120)

[BUG Vulkan Nvidia noise bug](https://www.shadertoy.com/view/ttjcRW) - [fixed already](https://forums.developer.nvidia.com/t/vulkan-shader-bug-can-someone-confirm-is-this-only-my-bug/140392), but very weird - basically all shaders that use fract and mod was bugged in Nvidia Vulkan for 5 years and no one noticed... Videos of this bug [1](https://youtu.be/hBbI2rw18ew) [2](https://youtu.be/KEqHarMBmOU)

[BUG floatBitsToUint comp vs real](https://www.shadertoy.com/view/tlfBRB) - related to blog post [Float precision on GPU, bugs/features](https://arugl.medium.com/float-precision-on-gpu-bugs-features-178ddd030f)

[BUG OpenGL Nvidia high GPU usage](https://www.shadertoy.com/view/tdfGWS) - Nvidia OpenGL compiler bug

[BUG smoothstep( 1, 0 ,0) specs](https://www.shadertoy.com/view/tdf3zf) - `smoothsep` on GPU does not follow specs, when on CPU it does.

[BUG Vulkan Nvidia sin hash](https://www.shadertoy.com/view/stK3WG) - sin hash broken in Vulkan on Nvidia, when in OpenGL everything fine.



## *Not a Bug*, its expected behavior but it still weird

[BUG 32-bit float precision](https://www.shadertoy.com/view/sllXW8) - related to blog post [Float precision on GPU, bugs/features](https://arugl.medium.com/float-precision-on-gpu-bugs-features-178ddd030f)

[BUG 32-bit float precision test](https://www.shadertoy.com/view/ftXXW4) - one of bugs related to *length of normalized vector not equal 1*

[BUG GPU driver unroll prediction](https://www.shadertoy.com/view/NlXXWS) - using break in loop adds precision to operation, expected because shader compiler compile shader diferently for each case.

[BUG GPU precision never 0](https://www.shadertoy.com/view/ftXSWB) - related to blog post [Float precision on GPU, bugs/features](https://arugl.medium.com/float-precision-on-gpu-bugs-features-178ddd030f)

[BUG Vulkan uint32 128 bytes push constant](https://www.shadertoy.com/view/3sXyW2) (does not work from WebGL, need modification read Shadertoy comment) - (not a bug) - just reminder for myself: in Vulkan - *Any member of a push constant block that is declared as an array must only be accessed with dynamically uniform indices*. If you use array as push const then result will be this - [screenshot 1](https://danilw.github.io/GLSL-howto/vulkan_sh_launcher/vk_amd_128pc.png), [screenshot 2](https://danilw.github.io/GLSL-howto/test_AMD_shader_ub/amd.png). Do not use array as push-constant in Vulkan. [khronos.org/registry/vulkan/specs](https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/chap15.html#interfaces-resources-pushconst)

[BUG Mip inconsistent Vulkan/Ogl](https://www.shadertoy.com/view/sltGWj) - When **sampler2D have *no mipmaps* but *has linear filter* and shader *has push_const*** - functions `texture` has some unpredictable result. (woks only in this specific shader(shader made from other large shader where I found this bug)). Bug does not work when shader does not have push_constants. Bug work on Shadertoy only in OpenGL mode `chrome.exe --use-angle=gl` and only when FBO has mipmaps. But in OpenGL this is "not a bug" or atleast it can be explained. [youtube video of this bug](https://youtu.be/OcpBVHMb88M)

[BUG Nvidia Vulkan UB behavior](https://www.shadertoy.com/view/fsc3RM) - weird UB behavior that result not same image depends on GPU, Vulkan only

[BUG Vulkan not equal to OpenGL](https://www.shadertoy.com/view/slsXzs) - weird UB behavior on GPU, on same GPU result not equal in OpenGL and Vulkan. Can be expected its UB.

[BUG cubemap rayDir to fragCoord](https://www.shadertoy.com/view/7l33W2) - related to [Cubemap GLSL shader debug functions](https://arugl.medium.com/cubemap-glsl-shader-debug-functions-3f4c659e7833)

[BUG normalize bugs](https://www.shadertoy.com/view/7lt3Rl) - related to [Cubemap GLSL shader debug functions](https://arugl.medium.com/cubemap-glsl-shader-debug-functions-3f4c659e7833)

[BUG tile rendering dFd test](https://www.shadertoy.com/view/NsXBWr) - not a bug, its expected behavior - dFd and texture functions return wrong result on edges when tile size not even. Related to [tile rendering example](https://www.shadertoy.com/view/tltBzM) and [advanced tile render](https://www.shadertoy.com/view/7ldXzf)

___

*Angle experience* - some of bug-related experience (unexpected behavior when your code is fine) - launching my [GLSL card game in ANGLE fist time (video)](https://youtu.be/uY15AZfesU4), Godot in ANGLE [FBO size jump (video)](https://youtu.be/rqmQ7EnDmb8), till 2019 (pre 76 version) Chrome had [broken instanced particles support](https://github.com/godotengine/godot/issues/28573).
___

### Driver updates:

Summer 2021 AMD update their driver (Linux and Windows, close and opensource) with fixing all my bugs that I reported during 2020-2021. Vulkan only fixes, OpenGL still very broken there. (that makes my [GLSL Auto Tetris](https://medium.com/geekculture/launching-619-thousand-tetris-on-gpu-their-rendering-and-a-simple-bot-f2449b607db1) and other my demos work in Vulkan on AMD GPUs)

Nvidia 510+ version (Linux and Windows, begin of 2022 update) - their SPIR-V compiler in driver fix some bugs with mat/vec/array index and some memory-[leaks (video)](https://youtu.be/puPplT1nPCI) from shaders also fixed, some bugs I listed also stop working in Vulkan(but still works in OpenGL), some bugs changed behavior.

