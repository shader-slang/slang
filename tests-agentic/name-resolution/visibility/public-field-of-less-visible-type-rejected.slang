//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/llm-generated/name-resolution/visibility.md#container-level-cap
//META: doc_section_digest=f97c340f9ed0d70698a682137262abf68c7dac2e0bc8a5cd8f0a2f7897f3b3f8
//META: purpose=A `public` field of a less-visible (internal) struct type inside a `public` aggregate is rejected with `use-of-less-visible-type`.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Container-level cap: the public field `member` references the
// internal type `IS`, so `checkVisibility` rejects with 30604.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct IS {}

public struct PS
{
    public IS member;
//CHECK:      ^^^^^^ references less visible type
//CHECK:      ^^^^^^ 'member' references less visible type 'IS'.
// CHECK: E30604
}
