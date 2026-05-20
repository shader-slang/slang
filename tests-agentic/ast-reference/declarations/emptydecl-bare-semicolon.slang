//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:09:57Z
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/ast-reference/declarations.md#nodes
//META: doc_section_digest=27f253003cd15aae74f1aea7cff6d2c4178b22d296301b6ed8e2aa0baf6c9884
//META: purpose=An EmptyDecl is a declaration that exists only to carry modifiers; a bare semicolon at module scope is accepted.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the documented role of EmptyDecl: a stand-alone `;` at
// module scope parses successfully (as a modifier-bearing empty
// declaration). The interpreter still runs the program.

//TEST:INTERPRET(filecheck=CHECK):

;

void main()
{
    //CHECK: ok
    printf("ok\n");
}
