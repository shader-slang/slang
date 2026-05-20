//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:09:57Z
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/ast-reference/declarations.md#nodes
//META: doc_section_digest=27f253003cd15aae74f1aea7cff6d2c4178b22d296301b6ed8e2aa0baf6c9884
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
