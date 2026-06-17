// Regression test for issue #10526: selecting a resource (texture/sampler/etc.)
// through control flow forces it into a function-local variable, which SPIR-V
// forbids (it would emit an OpStore/OpLoad/OpPhi of an opaque handle, producing
// Vulkan-invalid SPIR-V). Until such selections can be legalized, the compiler
// must reject this rather than emit invalid SPIR-V.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv

Texture1D<float> tex0, tex1;

[noinline]
Texture1D<float> returnHelper(Texture1D<float> a, Texture1D<float> b, bool useSecond)
{
    return useSecond ? b : a;
//CHECK:             ^ opaque type in local variable is not supported for SPIR-V
//CHECK:             ^ a resource or other opaque-typed value ('Texture1D') cannot be placed in a function-local variable for the SPIR-V target; this usually comes from selecting a resource with control flow (e.g. a '?:' or 'if'/'else') or returning one from a function
}

[shader("fragment")]
float main(bool useSecond) : SV_Target
{
    Texture1D<float> returned = returnHelper(tex0, tex1, useSecond);
    return returned[0];
}
