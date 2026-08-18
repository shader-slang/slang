//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/pipeline/04b-pre-link-passes.md#phase-d-non-essential-validation-stripping-and-finalization
//META: doc_section_digest=de3af7e62fdf97582267b3c674b87776ce5d49cf79a3d54ae3a770a3c679cad0
//META: purpose=An all-literal oversized shift is folded to a constant by Phase C, so no Lsh instruction is left for the Phase D shift-overflow validator to flag.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the carve-out that explains why the sibling shift-overflow test
// has to shift a `uniform`: the validator only sees shifts that survive
// constant propagation. The same 40-bit shift amount that draws a warning
// on a runtime operand is written here on a literal operand instead. The
// CHECKs assert both halves of the claim in one run -- the shift has become
// a stored constant zero and no shift instruction remains anywhere in the
// dump, and no overflow warning was reported.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    outBuf[0] = 1 << 40;
}

// No overflow warning is reported for the folded expression.
// CHECK-NOT: W41030
// CHECK: ### LOWER-TO-IR:
// The shift has already collapsed to the constant zero it evaluates to.
// CHECK: store(%{{[A-Za-z0-9_]+}}, 0 : Int)
// CHECK-NOT: lsh
