//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:48:38Z
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#module-and-namespace
//META: doc_section_digest=29b1e50e76ce09df697e30313ad1997122b36545720e82abde315d7de649b1b6
//META: purpose=A UsingDecl captures the current scope and injects the named namespace into the sibling-scope chain so the namespace's unqualified names become reachable at the use site.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// "`UsingDecl` captures the current scope at parse time and injects
// names at check time via the same sibling-scope mechanism." Verify
// that after `using namespace Foo`, `answer()` resolves unqualified
// at the call site through the sibling chain.

//TEST:INTERPRET(filecheck=CHECK):

namespace Foo
{
    int answer() { return 42; }
}

using namespace Foo;

void main()
{
    int v = answer();
    // CHECK: v=42
    printf("v=%d\n", v);
}
