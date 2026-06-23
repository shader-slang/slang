// Regression test for shader-slang/slang#6216: `[[vk::location]]` on a constant
// buffer / resource is silently ignored (the binding auto-allocates in
// declaration order); the compiler now errors (E39021) and points at
// `[[vk::binding]]`. Non-exhaustive FileCheck on `-target spirv`.

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main

struct Matrices
{
    float4x4 mvp;
};

// Misuse on a constant buffer -> E39021. (Code and message render on separate
// lines, so match them with separate CHECK directives.)
// CHECK: error{{.*}}39021
// CHECK: is not allowed on 'matrices'
[[vk::location(0)]]
ConstantBuffer<Matrices> matrices;

// Same misuse on any resource, not just `ConstantBuffer`.
// CHECK: error{{.*}}39021
// CHECK: is not allowed on 'tex'
[[vk::location(1)]]
Texture2D tex;

// Legitimate varying use must NOT error (regression guard).
// CHECK-NOT: is not allowed on 'uv'

[shader("fragment")]
float4 main([[vk::location(0)]] float2 uv : TEXCOORD)
    : SV_Target
{
    return float4(uv, 0.0, 0.0);
}
