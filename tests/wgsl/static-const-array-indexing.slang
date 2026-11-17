//TEST:SIMPLE(filecheck=WGSL): -stage vertex -entry vertexMain -target wgsl
// This second directive hands the emitted WGSL to Tint (via wgsl-spirv-asm) so we validate that
// the output actually compiles, not just that the keyword flipped. It is exercised on CI where
// Tint is available, and is ignored where the downstream compiler is absent.
//TEST:SIMPLE(filecheck=SPV): -stage vertex -entry vertexMain -target wgsl-spirv-asm

// Regression test for issue #6747.
//
// A `static const` array that is indexed by a runtime value is emitted as a module-scope WGSL
// `const`. WGSL allows a concrete array value to be indexed by a runtime value (WGSL spec, "Array
// Access Expression"), so `positions[vertexID]` is valid. A `const` is not a variable, so the
// table does not count against WGSL's 8192-byte limit on private variables, which WebKit
// enforces.

static const float2 positions[] = {
    float2(-1.0f, -1.0f),
    float2( 1.0f, -1.0f),
    float2( 1.0f,  1.0f),
    float2(-1.0f,  1.0f)
};

// The constant array must not be emitted as a private variable...
// WGSL-NOT: var<private>
// ...but as a module-scope `const` that keeps its inline initializer.
// WGSL: {{^}}const positions{{.*}} : array<vec2<f32>, i32(4)> = array<vec2<f32>, i32(4)>(

struct VertexOutput
{
    float4 position : SV_POSITION;
};

VertexOutput vertexMain(uint vertexID : SV_VertexID)
{
    VertexOutput output;
    // Indexing the `const` with a runtime value is valid WGSL.
    // WGSL: positions{{.*}}[vertexID
    // SPV: OpEntryPoint
    output.position = float4(positions[vertexID], 0.0f, 1.0f);
    return output;
}
