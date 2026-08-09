//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#error-code-namespace
//META: doc_section_digest=c07669430fa1f58c4db53ae38c50d1523f8399ab4e7aa698054222ac63e9d41b
//META: purpose=A `#warning` directive renders the preprocessor-range code E15901 with the user message interpolated.
//META: intent=functional
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pin `user-defined-warning` (15901) to its rendered code. The
// directive's payload is interpolated via `~message`.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):
#warning "custom preprocessor warning"
/*CHECK:
 ^^^^^^^ E15901
 ^^^^^^^ #warning: "custom preprocessor warning"
*/
void main() {}
