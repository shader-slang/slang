//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:25:40Z
//META: source_commit=6b490bc8cb3cf11b356ddf70168c5609efa46341
//META: doc_ref=docs/generated/design/name-resolution/visibility.md#edge-cases-and-failure-modes
//META: doc_section_digest=f0fb19c83c1fe744cfc27441df1ce1770502bc22a362ebfb8484f2adaed2edb9
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
