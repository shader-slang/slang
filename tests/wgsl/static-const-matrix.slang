//TEST:SIMPLE(filecheck=WGSL): -stage vertex -entry vertexMain -target wgsl
// Validate the emitted WGSL through Tint on CI (ignored where the downstream compiler is absent).
//TEST:SIMPLE(filecheck=SPV): -stage vertex -entry vertexMain -target wgsl-spirv-asm

// Regression test for issue #6747 — a `static const` MATRIX is NOT converted to `var<private>`.
//
// Only `static const` array globals are converted: a WGSL `const` value of array type may only
// be indexed by a const-expression. A matrix value, like a vector value, IS runtime-indexable in
// WGSL, so a `static const` matrix stays a `const` and is emitted as an inline value — it must
// NOT be promoted to `var<private>`. The `wgsl-spirv-asm` directive confirms Tint accepts the
// runtime row-index on CI; if it ever rejected it, matrices would be an instance of the same bug
// to convert too.

static const float2x2 M = {
    {1.0f, 2.0f},
    {3.0f, 4.0f}
};

// The matrix must NOT be promoted to a module-scope private variable. Placed before the positive
// check so the negative covers the whole globals region (where a promoted decl would appear).
// WGSL-NOT: var<private>{{.*}}mat2x2

struct VertexOutput
{
    float4 position : SV_POSITION;
};

VertexOutput vertexMain(uint i : SV_VertexID)
{
    VertexOutput output;
    // Runtime row-index of the matrix: it emits as an inline `mat2x2<...>(...)[i]`, not as a
    // `var<private>` referenced by name.
    // WGSL: mat2x2{{.*}}[i
    // SPV: OpEntryPoint
    output.position = float4(M[i % 2], 0.0f, 0.0f);
    return output;
}
