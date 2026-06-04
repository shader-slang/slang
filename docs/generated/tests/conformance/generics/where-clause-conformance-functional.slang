//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:43+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/generics.md#description
//META: doc_section_digest=7b3e1c2f4d8a0e9f5b2c6d1e3f7a4b8c0d2e6f9a1b5c3e7f0d4a8b2c6e0f4a
//META: purpose=A `where` clause conformance constraint (as opposed to an inline `T : IFoo` constraint) is accepted and enforces the same restriction.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a conformance constraint in a `where` clause is equivalent to an inline constraint.
// `extractValue<T>(T x) where T : IValued` calls `x.getValue()` on a Valued instance;
// CHECK asserts 123, reachable only if the where-clause constraint unlocked the interface method.

//TEST:INTERPRET(filecheck=CHECK):

interface IValued { int getValue(); }

struct Valued : IValued
{
    int _v;
    int getValue() { return _v; }
}

// Constraint in `where` clause rather than inline
func extractValue<T>(T x) -> int where T : IValued
{
    return x.getValue();
}

void main()
{
    Valued v;
    v._v = 123;
    // CHECK: 123
    printf("%d\n", extractValue(v));
}
