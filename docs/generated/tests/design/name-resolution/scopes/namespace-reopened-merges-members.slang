//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#sibling-scopes
//META: doc_section_digest=745e90bcc2440ac2accf9529f203ae4435a595ab30563e368559c64f8e6f978e
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
