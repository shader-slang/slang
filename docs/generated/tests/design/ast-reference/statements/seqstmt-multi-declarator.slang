//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/statements.md#blockstmt-and-seqstmt
//META: doc_section_digest=286b56c30cbbe8b637fe3e2feda9744698b33bf0a2434728ce750ab093cc9caf
//META: purpose=A SeqStmt bundles several statements where a single Stmt slot is required; the canonical surface form is multiple declarators in one declaration (`int a, b;`) used in statement position.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies several statements can fill a slot that holds only one:
// `int a = 3, b = 4;` in statement position introduces two independent
// names. The CHECKs assert each name carries its own initializer.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int a = 3, b = 4;
    // CHECK: a=3
    printf("a=%d\n", a);
    // CHECK: b=4
    printf("b=%d\n", b);
}
