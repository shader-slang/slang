//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#diagnosticsink
//META: doc_section_digest=83632b726fecd52003cb9a007e201049bb33c66f077ab6265b07278009caca02
//META: purpose=Boundary: a `#pragma warning(pop)` with an empty warning-state stack is itself reported as E15611.
//META: intent=boundary
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the boundary of the warning-state stack the sink tracks per source:
// a `#pragma warning(pop)` with nothing pushed is itself diagnosable. The
// caret-anchored CHECK pins code 15611 and the unmatched-pop message at the
// pragma's column.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):
#pragma warning(pop)
//CHECK:        ^^^ E15611
//CHECK:        ^^^ detected #pragma warning(pop) with no corresponding #pragma warning(push)

void main() {}
