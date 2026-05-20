//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:30:00+00:00
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/llm-generated/syntax-reference/keywords-and-builtins.md#core-module-supplied-vocabulary
//META: doc_section_digest=a84f2b8e553f246fd0e2611d7bd357de1ceb312fa259d862c4056388b6d6a68a
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
