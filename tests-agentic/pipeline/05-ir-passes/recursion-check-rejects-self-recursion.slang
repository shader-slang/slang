//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/pipeline/05-ir-passes.md#linking-and-validation
//META: doc_section_digest=61134c3253681818e651a9810123d97ae5bcc929737f8bed5d159bd896cf3248
//META: purpose=The check-recursion validation pass diagnoses unsupported recursion in the IR.
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A directly self-recursive function triggers an E55201 error from
// the check-recursion validation pass listed in the "Linking and
// validation" category.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -stage compute -entry main

uniform RWStructuredBuffer<int> buf;

int fact(int n)
{
    return n <= 1 ? 1 : n * fact(n - 1);
//CHECK:                        ^ recursion not allowed
//CHECK:                        ^ recursion detected in call to 'fact', but the current code generation target does not allow recursion.
}

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = fact(5);
}
