//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/generated/design/name-resolution/visibility.md#container-level-cap
//META: doc_section_digest=f97c340f9ed0d70698a682137262abf68c7dac2e0bc8a5cd8f0a2f7897f3b3f8
//META: purpose=A synthesized `$init` for a `public` struct that contains an `internal`-typed field is rejected with `references less visible type`.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Container-level cap: a public aggregate's synthesized initializer
// is treated as part of the aggregate's signature. With an internal
// field of type `IS`, the synthesized `$init` references a less
// visible type and `checkVisibility` rejects.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct IS {}

public struct PS
//CHECK:      ^^ references less visible type
//CHECK:      ^^ '$init' references less visible type 'IS'.
// CHECK: E30604
{
    public IS member;
}
