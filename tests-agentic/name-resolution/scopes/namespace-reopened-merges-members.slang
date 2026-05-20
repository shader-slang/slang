//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/name-resolution/scopes.md#sibling-scopes
//META: doc_section_digest=ed7fdf88e5d58ccf747775f1dfc045814145bf33056dcbfb7bd439f00b7e50cb
//META: purpose=Re-opening the same namespace name reuses the existing NamespaceDecl and links siblings; members from both declarations are reachable via the same qualified name.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// "Multiple `namespace Foo {}` declarations of the same logical
// namespace" is sibling-scope use case 3 in the doc. A second
// `namespace Foo` re-uses the first NamespaceDecl; both members are
// reachable through the same `Foo::` qualification.

//TEST:INTERPRET(filecheck=CHECK):

namespace Foo {
    int a() { return 1; }
}

namespace Foo {
    int b() { return 2; }
}

void main()
{
    int s = Foo::a() + Foo::b();
    // CHECK: s=3
    printf("s=%d\n", s);
}
