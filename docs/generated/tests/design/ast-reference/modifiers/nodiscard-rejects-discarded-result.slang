//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#mutability--autodiff-annotations
//META: doc_section_digest=a162d3ea93755dfa0e856855015bcc93b11eb24c29ad4e2f7cda299e7c6be3b7
//META: purpose=A call to a `[NoDiscard]` function whose result is discarded is diagnosed.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the enforced half of the rule: the attribute flags a function whose
// result must not be thrown away, so a call used as a bare expression statement
// is an error. CHECK pins the discarded-result error code on that statement.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry main -stage compute

[NoDiscard]
int triple(int x)
{
    return x * 3;
}

RWStructuredBuffer<int> outBuf;

[numthreads(1,1,1)]
void main()
{
    triple(4);
    //CHECK: E30059
    outBuf[0] = 1;
}
