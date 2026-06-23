// vk-location-on-cbuffer.slang
//
// Regression test for shader-slang/slang#6216.
//
// `[[vk::location]]` is only meaningful on a varying (stage input/output)
// parameter. Placing it on a constant buffer used to be silently ignored, so
// the buffer's binding was auto-allocated in declaration order and the user got
// a surprising binding. We now emit a warning pointing them at the correct
// attribute, `[[vk::binding]]`.
//
// (The warning is reported twice because, for an entry-point compile, the
// global parameter is laid out once for the global scope and once for the
// entry-point program. This matches the pre-existing behavior of sibling
// parameter-binding warnings such as `register-without-vulkan-binding`.)

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry main -stage fragment

struct DrawParams
{
    float4 color;
};

struct Matrices
{
    float4x4 mvp;
};

// Declared first, so without any explicit binding 'drawParams' auto-allocates 0.
ConstantBuffer<DrawParams> drawParams;

// `[[vk::location]]` on a constant buffer is the misuse from #6216: it does not
// set the binding (that needs `[[vk::binding]]`), so it is ignored.
[[vk::location(0)]]
/*CHECK:
      ^^^^^^^^ vk::location ignored on non-varying parameter
      ^^^^^^^^ '[[vk::location(...)]]' is ignored on 'matrices', which is not a varying (stage input/output) parameter; use '[[vk::binding(...)]]' to set the binding of a constant buffer or resource.
      ^^^^^^^^ vk::location ignored on non-varying parameter
      ^^^^^^^^ '[[vk::location(...)]]' is ignored on 'matrices', which is not a varying (stage input/output) parameter; use '[[vk::binding(...)]]' to set the binding of a constant buffer or resource.
*/
ConstantBuffer<Matrices> matrices;

[shader("fragment")]
float4 main() : SV_Target
{
    return mul(matrices.mvp, drawParams.color);
}
