---
layout: user-guide
permalink: /user-guide/cuda-target-specific
---

# CUDA/C++-Specific Functionalities

This chapter provides information for CUDA- and C++/CPU-specific functionalities and
behaviors in Slang. These two targets share the same C-style source emitter, so several
behaviors documented here apply to both.

## Performance

### Limitation: multi-component swizzle reads re-evaluate their base expression (tracked by [#12073](https://github.com/shader-slang/slang/issues/12073))

> This is a current, to-be-resolved limitation. It is tracked by
> [shader-slang/slang#12073](https://github.com/shader-slang/slang/issues/12073) and this
> note will be retired once the emitter fix lands.

When Slang emits a multi-component vector swizzle read such as `.rgb` or `.xyz` for the CUDA
or C++/CPU target, it synthesizes the result as a per-component constructor and re-emits the
swizzle's _base expression once per component_. Consider:

```hlsl
float3 s = src[q].rgb;   // src[q] is a texture/buffer load
```

On CUDA this emits (schematically):

```cpp
float3 s = float3{ tex(src, q).x, tex(src, q).y, tex(src, q).z };
```

The base `src[q]` — a texture or buffer load — is folded into each use site, so the load is
issued three times (`N` times for an `N`-component swizzle) rather than once. In a hot loop
this multiplies the memory traffic of the swizzled reads, and can make `float3` arithmetic
noticeably slower than the equivalent `float4` or scalar formulation. A reported neighborhood
kernel ran roughly 3x slower with a `float3` loop accumulator than with an identical `float4`
one on the CUDA target.

The SPIR-V (Vulkan) and HLSL targets are unaffected: SPIR-V lowers the swizzle to a single
`OpVectorShuffle` and HLSL emits a native `.xyz`, so in both the base is evaluated once. The
penalty is therefore specific to the CUDA and C++/CPU emit path, and it only bites when the
swizzle base is a non-trivial expression (a texture/buffer load or a function-call result); a
swizzle whose base is already a local variable is free to re-read and shows no penalty.

**Interim workarounds** (until [#12073](https://github.com/shader-slang/slang/issues/12073) is
fixed):

- Bind the load to a local variable first, then swizzle the local:
  `float4 s = src[q]; float3 rgb = s.rgb;` — the local is register-resident, so re-reading its
  components is free.
- Prefer reading individual components off a `float4` (`s.r`, `s.g`, `s.b`) over a `.rgb`
  swizzle in hot paths.
- Accumulate in `float4` (letting the extra lane ride along) or in named scalars rather than
  `float3` when the value originates from a load.

> #### Note
>
> This is a codegen (register/traffic) concern, distinct from the host/kernel memory-layout
> differences for CUDA and C++/CPU covered in
> [Matrix Layout](a1-01-matrix-layout.md). It does not change the in-memory layout of `float3`.
