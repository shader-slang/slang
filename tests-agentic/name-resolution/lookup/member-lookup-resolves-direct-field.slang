//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:48:38Z
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/name-resolution/lookup.md#member-lookup
//META: doc_section_digest=68a2d43cee4f2785028c9292caeaac660921efdc95b8ec2b5eface8a8b89a0dc
//META: purpose=lookUpMember on a DeclRefType resolves a direct field of the type via the underlying decl.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// lookUpMember dispatches on the type shape; for a plain
// `DeclRefType` (a struct value) it recurses into the underlying
// decl. Verify that `obj.field` resolves to the direct field.

//TEST:INTERPRET(filecheck=CHECK):

struct Point
{
    int x;
    int y;
}

void main()
{
    Point p;
    p.x = 11;
    p.y = 22;
    // CHECK: p=(11,22)
    printf("p=(%d,%d)\n", p.x, p.y);
}
