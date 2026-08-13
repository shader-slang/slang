//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#linking-and-validation
//META: doc_section_digest=74d850d9a54f442aac4cfdea60e05bde1723fd77cf6e54f9a6d3ec72cade3289
//META: purpose=Negative: the check-optional-none-usage pass rejects `.value` access on an `Optional<int>` declared with an explicit `= none` initializer (alternative shape to the default-constructed case).
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Alternative input shape for check-optional-none-usage: the
// optional is initialized explicitly with `= none`. The pass still
// flags the always-none read.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -stage compute -entry main

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Optional<int> o = none;
    buf[0] = o.value;
//CHECK:       ^^^^^ accessing .value on an Optional<int> that is always 'none'
//CHECK: E41027
}
