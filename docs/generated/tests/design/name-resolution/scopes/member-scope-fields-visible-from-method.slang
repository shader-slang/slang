//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#scope-bearing-ast-nodes
//META: doc_section_digest=ef276a3e56c4c6414ba8d252772bf603483822a32f729a0c9feb3df3959bc61a
//META: purpose=AggTypeDecl/StructDecl introduces a scope; a method body declared inside the struct can refer to the struct's fields by unqualified name.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// AggTypeDecl is listed as scope-bearing. The method's scope chain
// passes through the StructDecl, so unqualified `x` and `y` resolve
// to fields.

//TEST:INTERPRET(filecheck=CHECK):

struct Pair {
    int x;
    int y;
    int sum() { return x + y; }
}

void main()
{
    Pair p = {3, 5};
    int s = p.sum();
    // CHECK: s=8
    printf("s=%d\n", s);
}
