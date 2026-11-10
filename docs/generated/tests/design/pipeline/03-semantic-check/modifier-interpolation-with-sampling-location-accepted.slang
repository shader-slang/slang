//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:36:14Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#modifier-validation
//META: doc_section_digest=6d3d353f7acb30664cc90831179bf06d5f613a7e643d2f70a23548558a35ee35
//META: purpose=An interpolation mode combines with a sampling location because they are in separate conflict groups, so `noperspective centroid` on one field is accepted.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the other side of the group split: `noperspective` is in the
// interpolation group and `centroid` in the sampling-location group, so
// the pair raises no E31202. The test compiles to HLSL and CHECK asserts both
// modifiers survive on the emitted field (HLSL emission lists centroid first), which cannot happen if the checker
// had rejected the declaration.

//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry main -stage fragment

struct V
{
    noperspective centroid float4 c : COLOR;
}

[shader("fragment")]
float4 main(V v) : SV_Target
{
    return v.c;
}
// CHECK: centroid noperspective float4 {{[A-Za-z0-9_]+}} : COLOR
