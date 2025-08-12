# DO NOT USE SIN HASH
___

## Short:

*Sin-hash is not stable on "any scale" and there no way to fix it.*

- **Use fract hash** - **https://www.shadertoy.com/view/4djSRW**
- *Or int hash - but int hash have exact same behavior as fract hash, when fract hash is faster on GPU.*
- But - int and fract hash *can still be "broken"*

## Read [Hash Noise in GPU Shaders](https://arugl.medium.com/hash-noise-in-gpu-shaders-210188ac3a3e) - there lots of screenshots

*(notice on screenshots below - use `sin(large_numbers)` by itself is broken - not just in hash)*

___

### Fix (not always needed) to inconsistent fract hash:

```C
// not always needed, and not always fix
#define FIX_FRACT_HASH 1000.

float hash12(vec2 p)
{
#ifdef FIX_FRACT_HASH
    p = sign(p)*(floor(abs(p))+floor(fract(abs(p))*FIX_FRACT_HASH)/FIX_FRACT_HASH);
#endif
    vec3 p3  = fract(vec3(p.xyx) * .1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}

```

___

| Screenshot, click to open | Bug link |
|-------------|------------|
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
