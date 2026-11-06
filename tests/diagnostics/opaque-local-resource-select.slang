// `specializeResourceUsage` exposes `returnHelper`'s conditional texture result at its call site,
// even with `[noinline]`. `specializeResourceSelections` must then branch to fetch from `tex0` or
// `tex1` and merge the float result, without storing a texture in a function-local variable.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -validate-ir -validate-ir-detailed
//CHECK: OpBranchConditional
//CHECK: OpImageFetch
//CHECK: OpImageFetch

Texture1D<float> tex0, tex1;

[noinline]
Texture1D<float> returnHelper(Texture1D<float> a, Texture1D<float> b, bool useSecond)
{
    return useSecond ? b : a;
}

[shader("fragment")]
float main(bool useSecond) : SV_Target
{
    Texture1D<float> returned = returnHelper(tex0, tex1, useSecond);
    return returned[0];
}
