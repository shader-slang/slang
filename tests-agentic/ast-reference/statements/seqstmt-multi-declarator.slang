//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/ast-reference/statements.md#blockstmt-and-seqstmt
//META: doc_section_digest=71e446148629b488a053ae16dc5e7aee850ecd11b223e54fbd724907831ae599
//META: purpose=A SeqStmt bundles several statements where a single Stmt slot is required; the canonical surface form is multiple declarators in one declaration (`int a, b;`) used in statement position.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says SeqStmt "exists so that several statements can be
// presented where a single `Stmt` slot is required, without
// introducing a new scope". The canonical user-visible source for this
// is a multi-declarator declaration in statement position. We verify
// that both names are independently visible and carry their own
// initializers.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int a = 3, b = 4;
    // CHECK: a=3
    printf("a=%d\n", a);
    // CHECK: b=4
    printf("b=%d\n", b);
}
