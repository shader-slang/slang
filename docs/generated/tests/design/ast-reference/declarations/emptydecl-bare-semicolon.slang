//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#nodes
//META: doc_section_digest=f9721d75e32e3ac81b4ecdd86415227aa444d40f535b1e70e943f2f43e1c5bc0
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
