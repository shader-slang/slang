//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#inputs-and-outputs
//META: doc_section_digest=325192a5d2be9d9ec0f3812a36230d5c900a48f1c7af5d97ada17cd1926349b1
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
