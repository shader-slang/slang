//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/visibility.md#edge-cases-and-failure-modes
//META: doc_section_digest=64695a6d6e802521eed047636c41af1fa4d74dcb3b965fc47f17d6f2df4176e8
//META: purpose=A repeated visibility keyword (`public public`) is caught by the conflict-group check as `duplicate-modifier` (31202), not by the 36005 invalid-placement path.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies a repeated visibility keyword routes to the duplicate-modifier
// conflict-group rejection rather than the invalid-placement diagnostic.
// The CHECKs pin E31202 on the `public public` declaration; each
// duplicate-modifier diagnostic emits a title row and a span row, both
// carrying the code, so both rows are matched.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

public public int twice() { return 2; }
//CHECK: E31202
//CHECK: E31202
