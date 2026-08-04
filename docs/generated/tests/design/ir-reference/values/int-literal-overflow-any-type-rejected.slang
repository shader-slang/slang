//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ir-reference/values.md#literal-payload-encoding
//META: doc_section_digest=5e869d77d32b7f1e8abd286a5564168a9cb5d9a145cb0faea9dcc4c12e8e73e3
//META: purpose=Negative boundary: an integer literal larger than the documented `int64` MAX (99999999999999999999) is rejected with "integer literal is too large to be represented in any integer type" — the literal payload encoding has finite range.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's "literal payload encoding" claim implies the inline
// payload is finite-width. A literal larger than any integer type
// can represent fires three frontend signals: the "too large"
// hard error, the conversion overflow warning, and a follow-up
// "does not fit in type 'int'" warning. We check just the
// canonical "too large" message and use `non-exhaustive` to skip
// the related downstream signals.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -stage compute -entry main

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    int x = 99999999999999999999;
//CHECK:    ^^^^^^^^^^^^^^^^^^^^ integer literal is too large to be represented in any integer type
// CHECK: E10012
    buf[0] = x;
}
