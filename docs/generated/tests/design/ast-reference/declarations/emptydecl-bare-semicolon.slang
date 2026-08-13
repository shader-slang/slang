//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#nodes
//META: doc_section_digest=2b1e19c46f66c272b88b6086898b21a84533837b5f2bf1884cde7b0ab677590a
//META: purpose=An EmptyDecl is a declaration with no content of its own; a bare semicolon at module scope is accepted.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a stray `;` at module scope parses as an empty declaration
// rather than a syntax error. The program still runs; the CHECK asserts the
// entry point's output, which only prints if parsing completed.

//TEST:INTERPRET(filecheck=CHECK):

;

void main()
{
    //CHECK: ok
    printf("ok\n");
}
