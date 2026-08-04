//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#nodes
//META: doc_section_digest=1e2fffffe548d3dd55e164e2d482dadafc2362dd7c8b9d5b1675743c8d5fa05c
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
