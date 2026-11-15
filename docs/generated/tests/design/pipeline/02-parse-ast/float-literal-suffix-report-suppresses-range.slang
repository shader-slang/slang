//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#failure-modes
//META: doc_section_digest=35a0377e8327c387000594e690000a91232b821b59188e50e4c846f99b749334
//META: purpose=The parser reports at most one diagnostic per floating-point literal, so `1e400q` — an unrecognized suffix on a significand that also overflows — draws only the suffix report and the range report is suppressed.
//META: intent=negative
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the one-report-per-literal rule at the only input shape that can
// trip two of its conditions at once. The bare literal `1e400` overflows and
// draws the unrepresentable warning on its own; adding the unrecognized suffix
// `q` makes the classification arm fire first, and the guard flag then blocks
// the range arm behind it. Exhaustive mode is what turns that into an
// assertion: the single annotated suffix error is the *only* diagnostic, so a
// regression that let the range warning through as well would fail the test.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    let a = 1e400q;
//CHECK:    ^^^^^^ error E39999
//CHECK:    ^^^^^^ invalid suffix 'q' on floating-point literal
    printf("a=%d\n", int(a));
}
