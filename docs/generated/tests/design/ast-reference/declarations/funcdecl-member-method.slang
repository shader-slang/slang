//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#nodes
//META: doc_section_digest=1e2fffffe548d3dd55e164e2d482dadafc2362dd7c8b9d5b1675743c8d5fa05c
//META: purpose=A FuncDecl declared as a struct member becomes a method; calling the method dispatches through the receiver.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a FuncDecl inside an AggTypeDecl is reachable as a
// method; `this` inside the body refers to the receiver value.

//TEST:INTERPRET(filecheck=CHECK):

struct V
{
    int x;
    int doubled() { return x * 2; }
}

void main()
{
    V v;
    v.x = 21;
    //CHECK: 42
    printf("%d\n", v.doubled());
}
