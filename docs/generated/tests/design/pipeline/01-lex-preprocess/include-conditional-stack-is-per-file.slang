//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#stack-of-input-streams
//META: doc_section_digest=88580aa07a3dec484079754b6c3a0de7cacb1855ddd5d8fcd2684bb8080d8b3f
//META: purpose=Each `#include`d file gets its own stack of conditional state, so a `#if` opened in the header is diagnosed against the header at its end of file and an `#endif` in the parent has no conditional to close.
//META: intent=functional
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that conditional state is per-`InputFile` rather than
// global to the preprocessor. The included header opens `#if 1` and
// never closes it; the parent immediately writes `#endif`. If the two
// files shared one conditional stack, the parent's `#endif` would pair
// with the header's `#if` and the file would compile cleanly. Two
// diagnostics prove they did not pair: the header is reported for
// reaching end of file with a conditional still open (E15000), and the
// parent is reported for an `#endif` with nothing to close (E15001).

//TEST:SIMPLE(filecheck=CHECK):-target cpp -entry main -stage compute

#include "include-conditional-stack-is-per-file.slang.h"
#endif

// CHECK-DAG: E15000
// CHECK-DAG: E15001

void main() {}
