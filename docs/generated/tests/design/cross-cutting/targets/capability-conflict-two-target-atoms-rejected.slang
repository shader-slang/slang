//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/targets.md#definition-forms
//META: doc_section_digest=d6d360ed810fe20f1543f761a89b7a194383364a9db523edf2e823b33ddfb19e
//META: purpose=A `[require(hlsl, glsl)]` conjunction populates the `target` keyhole with two different atoms, which is an incompatible conjunction, and is rejected with `E36111: disallowed capability`.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that conjoining two atoms that populate the same keyhole
// is an incompatible conjunction. `hlsl` and `glsl` are both key
// atoms of the `target` keyhole, so `[require(hlsl, glsl)]` cannot
// name any reachable configuration; CHECK asserts the require
// resolves to the `Invalid` capability and is rejected with E36111,
// which is only reachable if the conjunction is treated as
// incompatible rather than as a disjunction.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry main -stage compute

[require(hlsl, glsl)]
/*CHECK:
 ^^^^^^^ disallowed capability
 ^^^^^^^ 'require' resolves into a disallowed `Invalid` Capability.
*/
// CHECK: E36111
void helper() {}

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    helper();
    outBuf[tid.x] = 1;
}
