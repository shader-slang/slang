// Resource-output specialization exposes the choice returned by `returnHelper` in its caller.
// Selection specialization must then dispatch the fetch and merge its ordinary result, even
// though the programmer requested that the helper not be inlined. No opaque local is needed.

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
