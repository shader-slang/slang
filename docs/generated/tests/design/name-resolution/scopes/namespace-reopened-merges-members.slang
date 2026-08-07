//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#sibling-scopes
//META: doc_section_digest=1730544025501f67efee33835c81f561c62ac7733b5e89832c526106e61b012f
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
