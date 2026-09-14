//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/visibility.md#per-keyword-semantics
//META: doc_section_digest=7d96ab9817d6f6251e2148b7d4d9959fb9ec73041354e919db537f46458d416e
//META: purpose=The 2026 aggregate-inheritance rule composes transitively, so an unmodified aggregate nested in an `internal` aggregate is itself `internal` and rejects a `public` member.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the 2026 rule keys off the parent's *effective* visibility
// rather than an explicit modifier. `Inner` carries no modifier, so under
// 2026 it inherits `internal` from `Outer`; the module default is `public`,
// so without the transitive step `Inner` would have been public and its
// `public` member would be legal. The CHECK pins the E30601 rejection of
// `g`, which can only fire if `Inner` resolved to `internal` transitively.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-std 2026

public module nestedAggregateTransitive2026;

internal struct Outer
{
    struct Inner
    {
        public void g() {}
//CHECK:            ^ visibility higher than parent
//CHECK:            ^ 'g' cannot have a higher visibility than 'Inner'.
// CHECK: E30601
    }
}
