//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#generictypeconstraintdecl-as-an-interface-requirement
//META: doc_section_digest=3c3b338a49679f1b453ba7815d8deac7d31be7956bc0fe69e33cdab3863157ae
//META: purpose=The reversed pack-count spelling `N == countof(Pack)` is recognized only to diagnose that `countof(...)` must appear on the left.
//META: intent=negative
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the reversed pack-count spelling is rejected with a dedicated
// message rather than being silently reinterpreted as an unrelated type
// constraint. The CHECKs pin the diagnostic at the `countof` token and the
// suggested rewrite, which is what makes the left-hand-side rule observable.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void reversed<let N : int, each T>() where N == countof(T)
//CHECK:                                        ^^^^^^^ `countof(...)` pack-count constraint must be written with `countof(...)` on the left side
//CHECK:                                        ^^^^^^^ write this constraint as `countof(Pack) == IntExpr`
{
}

void main() {}
