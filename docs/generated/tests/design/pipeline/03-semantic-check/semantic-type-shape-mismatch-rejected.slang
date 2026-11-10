//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#shader-specific-checks
//META: doc_section_digest=73b0a54aa16472868adbc586d5469e6a6d226c42e5fd1a1ff6720178eede46b1
//META: purpose=A system-value semantic rejects a declared type with a different element count, so a scalar `float` (one element) is not valid for the four-element `SV_Position` semantic (E30701).
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the shape half of the compatibility rule: the element category
// matches here (both floating-point), so only the scalar-versus-vector shape
// test can reject it. CHECK pins E30701, whose message lists the vector forms
// the semantic accepts.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl

[shader("fragment")]
float4 main(float pos: SV_Position): SV_Target
//CHECK: E30701
{
    return float4(pos, pos, pos, 1);
}
