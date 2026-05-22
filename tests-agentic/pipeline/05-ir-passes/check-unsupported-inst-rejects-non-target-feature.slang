//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/pipeline/05-ir-passes.md#linking-and-validation
//META: doc_section_digest=61134c3253681818e651a9810123d97ae5bcc929737f8bed5d159bd896cf3248
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
