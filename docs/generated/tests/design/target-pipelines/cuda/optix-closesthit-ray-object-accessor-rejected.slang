//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/cuda.md#adjacent-targets
//META: doc_section_digest=869ac6b9c3da03d34224a4f31b9e0acb7f7288e722f39fcd9302911170d75b63
//META: purpose=CUDA/OptiX stage coverage excludes closesthit for the ray-object accessors, so ObjectRayOrigin in a closesthit entry point is rejected with E36107 unavailable features in entry point.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the negative half of the CUDA/OptiX stage-coverage claim:
// unlike `anyhit` and `intersection`, `closesthit` does not carry the
// capability for the ray-object accessors on this target family. The
// capability set is checked while the entry point is type-checked, so
// the compile stops before any CUDA text is emitted. The CHECK pins the
// unavailable-features code on the entry-point signature line, where the
// diagnostic is reported.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target cuda -entry chMain -stage closesthit

struct Payload
{
    float4 color;
};

[shader("closesthit")]
void chMain(inout Payload payload, in BuiltInTriangleIntersectionAttributes attribs)
//CHECK: E36107
{
    float3 o = ObjectRayOrigin();
    payload.color = float4(o, 1.0f);
}
