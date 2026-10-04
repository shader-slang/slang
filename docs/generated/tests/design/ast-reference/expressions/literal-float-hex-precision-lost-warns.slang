//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#literalexpr-family
//META: doc_section_digest=f15f833400e70f33bd4f151e90be63f4887ec974cd69a44d2663e7a0d2ca3bdb
//META: purpose=A hexadecimal float literal whose significand does not fit the suffix type is reported as warning E40019 and the truncated value is what lands in the literal.
//META: intent=boundary
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the third of the three literal diagnostics, the one that is about
// precision rather than magnitude: the value is in range, but the hex
// significand carries more bits than a `float` has. The parser does not round
// silently and it does not reject the program either - it warns and keeps the
// truncated conversion. CHECK pins E40019 on the literal.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -stage compute -entry computeMain

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = 0x1.123456789abcdefp0f;
//CHECK:              ^^^^^^^^^^^^^^^^^^^^^^ E40019
}
