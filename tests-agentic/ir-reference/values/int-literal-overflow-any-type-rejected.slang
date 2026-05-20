//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/llm-generated/ir-reference/values.md#literal-payload-encoding
//META: doc_section_digest=04423d40739a5430fb339507cc0da277b4a8a29153c176db3ec3d56a42a4070b
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
    buf[0] = x;
}
