//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=d245d3be9942f0847c45c138b4c2780e7df19ecb
//META: doc_ref=docs/llm-generated/pipeline/05-ir-passes.md#linking-and-validation
//META: doc_section_digest=61134c3253681818e651a9810123d97ae5bcc929737f8bed5d159bd896cf3248
//META: purpose=Negative: the check-optional-none-usage pass rejects `.value` access on an `Optional<int>` declared with an explicit `= none` initializer (alternative shape to the default-constructed case).
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Alternative input shape for check-optional-none-usage: the
// optional is initialized explicitly with `= none`. The pass still
// flags the always-none read.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -stage compute -entry main

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Optional<int> o = none;
    buf[0] = o.value;
//CHECK:       ^^^^^ accessing .value on an Optional<int> that is always 'none'
//CHECK: E41027
}
