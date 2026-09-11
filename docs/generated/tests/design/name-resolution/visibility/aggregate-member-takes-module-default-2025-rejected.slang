//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/visibility.md#per-keyword-semantics
//META: doc_section_digest=7d96ab9817d6f6251e2148b7d4d9959fb9ec73041354e919db537f46458d416e
//META: purpose=Under Slang 2025 the aggregate-inheritance rule does not apply, so an unmodified member of an `internal` aggregate takes the module default and is rejected as more visible than its parent.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the language-version gate on the aggregate-inheritance rule: the
// same source that compiles under 2026 must fall back to the module default
// under 2025. The module is `public`, so `base` and `doubled` become public
// members of an `internal` struct; the CHECKs pin the E30601 higher-than-
// parent rejection on each of them, which only fires if the 2026 rule was
// skipped.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-std 2025

public module aggregateMemberModuleDefault2025;

internal struct Counter
{
    int base;
/*CHECK:
        ^^^^ visibility higher than parent
        ^^^^ 'base' cannot have a higher visibility than 'Counter'.
*/
    int doubled() { return base * 2; }
/*CHECK:
        ^^^^^^^ visibility higher than parent
        ^^^^^^^ 'doubled' cannot have a higher visibility than 'Counter'.
*/
// CHECK: E30601
}

internal int run(Counter c) { return c.doubled(); }
