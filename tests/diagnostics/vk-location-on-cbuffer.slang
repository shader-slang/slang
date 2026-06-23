// vk-location-on-cbuffer.slang
//
// Regression test for shader-slang/slang#6216.
//
// `[[vk::location(N)]]` only adjusts the location of a *varying* (stage
// input/output) parameter. Placing it on a constant buffer or resource is
// silently ignored, so the binding is auto-allocated in declaration order and
// the user gets a surprising, order-dependent binding (the #6216 report). The
// compiler now warns (W39021) and points the user at `[[vk::binding(...)]]`.
//
// The warning is gated to the Khronos/WGSL targets that actually consume
// binding modifiers; `-target spirv` (the representative Vulkan target) is used
// here. Non-exhaustive FileCheck is used deliberately so the assertions do not
// couple to how many parameter-binding passes a referenced global happens to go
// through.

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main

struct Matrices
{
    float4x4 mvp;
};

// Misuse: `[[vk::location]]` on a constant buffer. The attribute is dropped and
// the binding still auto-allocates in declaration order -- warn. (The renderer
// prints the code and the message on separate lines, so they are matched by
// separate CHECK directives.)
// CHECK: warning{{.*}}39021
// CHECK: is ignored on 'matrices'
[[vk::location(0)]]
ConstantBuffer<Matrices> matrices;

// The misuse generalizes to any non-varying resource, not just `ConstantBuffer`.
// CHECK: warning{{.*}}39021
// CHECK: is ignored on 'tex'
[[vk::location(1)]]
Texture2D tex;

// Legitimate varying use: `[[vk::location]]` on a stage input is exactly what
// the attribute is for, so the `isVaryingParam` guard must NOT warn here. This
// is the regression guard against re-introducing a false positive on every
// correct varying use.
// CHECK-NOT: is ignored on 'uv'

[shader("fragment")]
float4 main([[vk::location(0)]] float2 uv : TEXCOORD)
    : SV_Target
{
    return float4(uv, 0.0, 0.0);
}
