//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/llm-generated/pipeline/03-semantic-check.md#inputs-and-outputs
//META: doc_section_digest=85c534d82e8549dc778af3fda4ed6b0174cc56ffa67072e5ed391a2e0a2d7858
//META: purpose=After checking, a DeclRef-bearing node points at the canonical decl; a name used after its declaration resolves to that decl and the call returns the expected value.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that calling a user-defined function works: name lookup
// resolves the `compute` call to the FuncDecl above main, and the
// program runs with the expected return value.

//TEST:INTERPRET(filecheck=CHECK):

int compute(int x) { return x * x + 1; }

void main()
{
    // CHECK: r=10
    printf("r=%d\n", compute(3));
}
