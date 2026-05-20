//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:48:38Z
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/name-resolution/lookup.md#member-lookup
//META: doc_section_digest=68a2d43cee4f2785028c9292caeaac660921efdc95b8ec2b5eface8a8b89a0dc
//META: purpose=Member lookup walks the inheritance facets to reach a method declared on an interface; the conforming struct's override is callable through the struct receiver directly.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc anchors member lookup on a DeclRefType to the inheritance
// walk. Verify that calling a method declared on an interface
// resolves via the conforming struct's own implementation when the
// receiver is the concrete struct type. (Companion to the existing
// 03-semantic-check generic test: this one uses a direct receiver,
// no generic indirection.)

//TEST:INTERPRET(filecheck=CHECK):

interface IFlag { int marker(); }

struct A : IFlag
{
    int marker() { return 42; }
}

void main()
{
    A a;
    // CHECK: r=42
    printf("r=%d\n", a.marker());
}
