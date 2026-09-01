//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#failure-modes
//META: doc_section_digest=045c5379570a64224b0f0d784c24bc57410eef152e105148ecb4e662cb3f9c88
//META: purpose=A decimal `ll` literal of INT64_MAX + 2 or above warns W40004 that it is too large for a signed integer type and stays unsigned.
//META: intent=boundary
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the far side of the signed/unsigned boundary, one step past the
// signed-minimum exception. INT64_MAX + 1 is silently unsigned because the
// negation form still needs it; INT64_MAX + 2 has no such use, so it both warns
// and stays unsigned. The warning is annotated here, and the "stays unsigned"
// half is what the accompanying arithmetic shows: subtracting the literal from
// itself in an unsigned 64-bit type yields 0 without the wrap a signed
// reinterpretation would produce.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    let big = 9223372036854775809ll;
//CHECK:      ^^^^^^^^^^^^^^^^^^^^^ warning E40004
    uint64_t stillUnsigned = big;
    printf("d=%d\n", int(stillUnsigned - big));
}
