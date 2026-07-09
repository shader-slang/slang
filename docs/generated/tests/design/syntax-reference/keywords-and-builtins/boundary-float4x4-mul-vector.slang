//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:30:00+00:00
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#core-module-syntax-declarations
//META: doc_section_digest=950d7a7efcea5da748ec49fa6499555097f3093abb07ac97945a51be1ce2339c
//META: purpose=Boundary: `float4x4` is the maximum-square matrix vocabulary and `mul` is documented as an HLSL intrinsic; combining them at the largest square boundary scales the input vector component-wise via a diagonal matrix.
//META: intent=boundary
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Probes the maximum-width square matrix together with the `mul`
// intrinsic. The diagonal matrix yields a known per-component scale
// of the input vector — easy to observe and impossible to fudge.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    float4x4 M = float4x4(
        1.0, 0.0, 0.0, 0.0,
        0.0, 2.0, 0.0, 0.0,
        0.0, 0.0, 3.0, 0.0,
        0.0, 0.0, 0.0, 4.0);
    float4 v = float4(1.0, 1.0, 1.0, 1.0);
    float4 r = mul(M, v);
    // CHECK: r=1 2 3 4
    printf("r=%d %d %d %d\n", int(r.x), int(r.y), int(r.z), int(r.w));
}
