//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/statements.md#declstmt-expressionstmt-and-emptystmt
//META: doc_section_digest=e05101999cc6555f3e7214b2fb06744dbaf16788905b25807e20594acfdd10d6
//META: purpose=An EmptyStmt is a bare `;` accepted in statement position; it carries no state and produces no effect.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies a bare `;` is a legal statement that produces no effect.
// Semicolons are interleaved between two side-effecting statements; the
// CHECK asserts the arithmetic result is unaffected, so the empty
// statements parsed and executed as no-ops.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int x = 1;
    ;
    ;
    x = x + 10;
    ;
    // CHECK: x=11
    printf("x=%d\n", x);
}
