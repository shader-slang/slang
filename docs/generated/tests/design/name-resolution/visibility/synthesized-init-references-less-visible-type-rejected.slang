//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/visibility.md#container-level-cap
//META: doc_section_digest=9489d5d7008e3b7339c769e32cc45730a497b35c5a531094faa03ce8571ca08f
//META: purpose=A synthesized `$init` for a `public` struct that contains an `internal`-typed field is rejected with `references less visible type`.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Container-level cap: a public aggregate's synthesized initializer
// is treated as part of the aggregate's signature. With an internal
// field of type `IS`, the synthesized `$init` references a less
// visible type and `checkVisibility` rejects.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct IS {}

public struct PS
//CHECK:      ^^ references less visible type
//CHECK:      ^^ 'PS.init' references less visible type 'IS'.
{
    public IS member;
//CHECK:      ^^^^^^ references less visible type
//CHECK:      ^^^^^^ 'member' references less visible type 'IS'.
}
