//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:35:39+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#sibling-scopes
//META: doc_section_digest=48e2e716cb98e52f251b42e53f41f2630e81baad625d5ff212284e8c3ac35e05
//META: purpose=Each body of a reopened namespace has its own Scope, and the checker attaches the other bodies' scopes as siblings, so code in a later body reaches an earlier body's member unqualified.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Sibling-scope use 3: each `namespace Foo { ... }` body is parsed with its
// own Scope (recorded in NamespaceDecl::lexicalScopes), and
// visitNamespaceDecl wires every body to the others. A call written inside
// the second body therefore finds `first()` from the first body with no
// `Foo::` qualification.

//TEST:INTERPRET(filecheck=CHECK):

namespace Foo {
    int first() { return 40; }
}

namespace Foo {
    int second() { return first() + 2; }
}

void main()
{
    // CHECK: v=42
    printf("v=%d\n", Foo::second());
}
