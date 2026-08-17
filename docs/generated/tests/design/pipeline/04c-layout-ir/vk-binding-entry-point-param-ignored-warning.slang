//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04c-layout-ir.md#explicit-vkbinding-on-entry-point-parameters
//META: doc_section_digest=d594d25adb697c7a448d8fd4e5af2ada74a9ee315fdd20515276d88cbf22323f
//META: purpose=A vk::binding on an entry-point parameter that the target cannot honor is reported with the unsupported-entry-point-parameter-modifier warning E38010.
//META: intent=negative
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that ignoring the attribute is diagnosed rather than silent.
// Compiling for HLSL, which is outside the target gate, must report
// E38010 against the annotated parameter; the caret annotation pins the
// diagnostic to the parameter name so a silently dropped attribute would
// fail the test.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[numthreads(1, 1, 1)]
void main([[vk::binding(3, 1)]] uniform StructuredBuffer<int> inBuf)
//CHECK:                                                      ^^^^^ modifier on entry point parameter is unsupported
//CHECK:                                                      ^^^^^ attribute '[[vk::binding(...)]]' on parameter 'inBuf' is unsupported on entry point parameters and will be ignored
{
    outBuf[0] = inBuf[0];
}
