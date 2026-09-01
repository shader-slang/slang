//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#literalexpr-family
//META: doc_section_digest=f15f833400e70f33bd4f151e90be63f4887ec974cd69a44d2663e7a0d2ca3bdb
//META: purpose=A float literal whose magnitude is below the range of its suffix type is reported as warning E40010 and the already-converted value zero is what lands in the literal.
//META: intent=boundary
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the under-range half of the literal-range rule, the mirror of the
// over-range case its sibling literal-float-out-of-range-diagnosed.slang
// covers. The parser stores what the conversion produced rather than clamping
// itself, and it reports the loss as a warning rather than an error, so the
// compile continues. CHECK pins E40010 on the literal; the warning severity is
// what lets the surrounding entry point still be emitted.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -stage compute -entry computeMain

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = 1e-60f;
//CHECK:              ^^^^^^ E40010
}
