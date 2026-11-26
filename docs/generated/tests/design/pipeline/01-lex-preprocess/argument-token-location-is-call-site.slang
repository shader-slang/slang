//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-09T10:40:15+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#source-location-preservation
//META: doc_section_digest=b376d677453ba50705d8234aa3572303b42f0c428f903a6b2d9d9b73a3df65a7
//META: purpose=An argument token keeps the call-site location it was lexed with, and a parameter used twice replays the same location, so the diagnostic points at the argument in the invocation rather than into the macro body.
//META: intent=functional
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Category 2 of the source-location rules. `TWICE(x)` uses its parameter
// twice; a diagnostic about the argument must point at `undeclared_arg` in
// the invocation, where category 1 (raw body tokens) would have pointed at
// one of the `(x)` in the definition.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

#define TWICE(x) ((x) + (x))

void main()
{
    int v = TWICE(undeclared_arg);
//CHECK:          ^^^^^^^^^^^^^^ undefined identifier
// CHECK: E30015
    printf("v=%d\n", v);
}
