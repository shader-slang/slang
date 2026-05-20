//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:09:57Z
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/ast-reference/declarations.md#nodes
//META: doc_section_digest=27f253003cd15aae74f1aea7cff6d2c4178b22d296301b6ed8e2aa0baf6c9884
//META: purpose=A DeclGroup wraps multiple declarations parsed as one group (int a, b;); each name is independently visible.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the documented role of DeclGroup. A single `int a, b;`
// produces two independent VarDecls inside a DeclGroup; both names
// are usable independently after the declaration.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int a, b;
    a = 3;
    b = 5;
    //CHECK: 8
    printf("%d\n", a + b);
}
