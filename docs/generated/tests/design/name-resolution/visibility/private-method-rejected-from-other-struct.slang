//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/generated/design/name-resolution/visibility.md#where-visibility-is-filtered
//META: doc_section_digest=d1b896b6a6adec252014d2d12ba87c258c20d4783523aeec02b3767c1a892991
//META: purpose=A `private` method of struct A is not visible from a method of an unrelated struct B; `isDeclVisibleFromScope` walks B's parents and never finds A.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `isDeclVisibleFromScope` for `Private` checks whether the
// requesting scope's parent chain contains A's enclosing aggregate.
// Two unrelated structs are not in each other's parent chain, so
// access is rejected.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct A
{
    private int secret() { return 1; }
}

struct B
{
    int peek(A a) { return a.secret(); }
//CHECK:                     ^^^^^^ declaration not accessible
//CHECK:                     ^^^^^^ 'secret' is not accessible from the current context.
// CHECK: E30600
}
