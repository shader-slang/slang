//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/visibility.md#edge-cases-and-failure-modes
//META: doc_section_digest=64695a6d6e802521eed047636c41af1fa4d74dcb3b965fc47f17d6f2df4176e8
//META: purpose=A `public` member inside an `internal` aggregate is rejected by `checkVisibility` with `decl-cannot-have-higher-visibility`.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Edge case: a `public` member of a non-public container fires
// diagnostic 30601 (`decl-cannot-have-higher-visibility`). The
// surrounding struct is `internal` (the modern default).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct Container
{
    public void leak() {}
//CHECK:        ^^^^ visibility higher than parent
//CHECK:        ^^^^ 'leak' cannot have a higher visibility than 'Container'.
// CHECK: E30601
}
