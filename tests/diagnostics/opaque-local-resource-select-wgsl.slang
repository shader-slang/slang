// Issue #10526: WGSL has the same restriction as Khronos targets — textures and
// samplers live in WGSL's `handle` address space, which the spec requires to be
// module-scope only. Selecting a resource through control flow into a local must
// therefore be rejected for `-target wgsl` too (not just SPIR-V/GLSL).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target wgsl

Texture2D<float> t0, t1;
SamplerState s;

[noinline]
Texture2D<float> sel(bool b)
{
    return b ? t1 : t0;
}

[shader("fragment")]
float main(bool b) : SV_Target
{
    return sel(b).Sample(s, float2(0, 0));
//CHECK:      ^ opaque type in local variable is not supported for Khronos targets
//CHECK:      ^ a resource or other opaque-typed value ('Texture2D') cannot be placed in a function-local variable for Khronos targets (SPIR-V/GLSL) or WGSL; this usually comes from selecting a resource with control flow (e.g. a '?:' or 'if'/'else') or returning one from a function
}
