//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#linking-and-validation
//META: doc_section_digest=74d850d9a54f442aac4cfdea60e05bde1723fd77cf6e54f9a6d3ec72cade3289
//META: purpose=Targeting a profile the entry-point requires features beyond is rejected by capability validation passes.
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Using `printf` on a target that does not support it triggers a
// validation diagnostic (the "Check unsupported inst" /
// "Late require capability" rows of the validation table). Specifically,
// `printf` is not available on GLSL out of the box without the
// debug-printf extension, so emitting to glsl fails with a capability
// error.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target glsl -stage compute -entry main -profile glsl_450

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
/*CHECK:
     ^^^^ profile implicitly upgraded
*/
// CHECK: W41012
{
    printf("debug message %d\n", buf[0]);
}
