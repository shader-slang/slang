//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#nodes
//META: doc_section_digest=1e2fffffe548d3dd55e164e2d482dadafc2362dd7c8b9d5b1675743c8d5fa05c
//META: purpose=A FuncDecl is an ordinary function declaration; two FuncDecls with the same name but different parameter types overload independently.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the documented role of FuncDecl as the ordinary function
// declaration. Two FuncDecls with the same name and different
// parameter types coexist; overload resolution dispatches on the
// argument type at the call site.

//TEST:INTERPRET(filecheck=CHECK):

int classify(int x)   { return 1; }
int classify(float x) { return 2; }

void main()
{
    //CHECK: 1 2
    printf("%d %d\n", classify(3), classify(2.5f));
}
