//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/ast-reference/statements.md#nodes
//META: doc_section_digest=9f32f2463c3b3150d88ac333b9a732766e6f477e8e541627f6f4db0faa939a24
//META: purpose=An EmptyStmt is a bare `;` accepted in statement position; it carries no state and produces no effect.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says EmptyStmt is "a bare `;`" with no additional state. We
// observe by placing a bare semicolon between two side-effecting
// statements: the program must still run both and produce the same
// result.

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
