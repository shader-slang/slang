//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/visibility.md#where-visibility-is-filtered
//META: doc_section_digest=a89f8b98a91c1e771a177ba6097fa77139369c98b2d45bac6f12e2be5bc2ee74
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
