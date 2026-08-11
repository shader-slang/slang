//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/control-flow.md#discard
//META: doc_section_digest=6a5608a206f750dd04fa2d1029419418beb4607b6026071a262da03fd96ab75b
//META: purpose=Negative: `discard` is fragment-only per the doc ("HLSL discard for fragment shaders") — using it from a compute entry point is rejected with E36107 "unavailable features in entry point" for the compute stage.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `discard` is fragment-only by the doc; a compute entry point using
// it should be rejected with E36107 before IR lowering produces any
// `discard` opcode.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv -entry main -stage compute

uniform float threshold;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main()
/*CHECK:
     ^^^^ unavailable features in entry point
*/
// CHECK: E36107
{
    if (threshold < 0.0)
        discard;
    buf[0] = threshold;
}
