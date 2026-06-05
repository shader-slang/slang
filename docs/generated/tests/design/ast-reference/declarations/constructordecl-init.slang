//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:09:57Z
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#nodes
//META: doc_section_digest=27f253003cd15aae74f1aea7cff6d2c4178b22d296301b6ed8e2aa0baf6c9884
//META: purpose=A ConstructorDecl is a __init declaration; calling the type as a function dispatches to the constructor and initializes the struct.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the documented role of ConstructorDecl. `__init(int v)`
// declares a constructor; the constructor-call syntax `Wrap(5)`
// invokes it and yields a struct whose field reflects the argument.

//TEST:INTERPRET(filecheck=CHECK):

struct Wrap
{
    int v;
    __init(int x) { v = x * 3; }
}

void main()
{
    Wrap w = Wrap(5);
    //CHECK: 15
    printf("%d\n", w.v);
}
