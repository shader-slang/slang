//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/statements.md#blockstmt-and-seqstmt
//META: doc_section_digest=fcf4d2f955e68033a3f7d41a57eb38d3b947a94caa962232e1c2c51d654a34f2
//META: purpose=An empty BlockStmt `{}` is accepted in statement position and has no observable effect; the surrounding state is preserved across the block.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the smallest block form — `{}` with no statements at all —
// is accepted in statement position and contributes nothing. The CHECK
// asserts a variable declared before the block still reads 42 after it,
// which holds only if the empty block parsed as a real statement and
// executed as a no-op.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int x = 42;
    {
    }
    // CHECK: x=42
    printf("x=%d\n", x);
}
