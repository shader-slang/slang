//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:40:00+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#module-and-namespace
//META: doc_section_digest=133885f40d47646386a7126573bda25629492d971ace7c1f5d02ba88572d11f6
//META: purpose=A namespace declared with a qualified name such as A::B resolves unqualified names against the enclosing namespace A, because each lexical scope of a namespace is wired to the same-named namespaces visible from its parent chain.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies scope wiring for `namespace A::B`. The body of `f` names
// `x()` without qualification; `x` lives in a separate `namespace A`
// block, so the reference resolves only if the lexical scope created
// for the qualified declaration reaches the merged `A`. CHECK asserts
// the value 5 flows through.

//TEST:INTERPRET(filecheck=CHECK):

namespace A { int x() { return 5; } }

namespace A::B
{
    int f() { return x(); }
}

void main()
{
    // CHECK: r=5
    printf("r=%d\n", A::B::f());
}
