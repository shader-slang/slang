//TEST:SIMPLE(filecheck=WGSL): -stage vertex -entry vertexMain -target wgsl

// Regression test for issue #6747.
//
// A `static const` array that is indexed by a runtime value must be emitted as a WGSL
// `var<private>`, not as a WGSL `const`. A WGSL `const` is a compile-time value (closer to
// C++ `constexpr`); the WGSL spec only allows a value of array type to be indexed by a
// const-expression, so `const positions[vertexID]` (a runtime index) is rejected by the
// WGSL validator with "The expression may only be indexed by a constant". `var<private>`
// keeps the same inline initializer but is addressable and therefore runtime-indexable.

static const float2 positions[] = {
    float2(-1.0f, -1.0f),
    float2( 1.0f, -1.0f),
    float2( 1.0f,  1.0f),
    float2(-1.0f,  1.0f)
};

// The constant array must not be emitted as a WGSL `const`...
// WGSL-NOT: const positions
// ...but as a private module-scope variable that keeps its inline initializer.
// WGSL: var<private> positions{{.*}} : array<vec2<f32>, i32(4)> = array<vec2<f32>, i32(4)>(

struct VertexOutput
{
    float4 position : SV_POSITION;
};

VertexOutput vertexMain(uint vertexID : SV_VertexID)
{
    VertexOutput output;
    // Indexing the global with a runtime value is exactly what a WGSL `const` forbids.
    // WGSL: positions{{.*}}[vertexID
    output.position = float4(positions[vertexID], 0.0f, 1.0f);
    return output;
}
