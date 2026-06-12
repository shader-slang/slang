//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:09:57Z
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#nodes
//META: doc_section_digest=27f253003cd15aae74f1aea7cff6d2c4178b22d296301b6ed8e2aa0baf6c9884
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
