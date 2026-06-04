//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/generated/design/name-resolution/visibility.md#edge-cases-and-failure-modes
//META: doc_section_digest=942497f445c2102c7a435609ccb26e1bc901622765b619863b12aa14a39e7af0
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
