//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#error-code-namespace
//META: doc_section_digest=5ef206950ef0e6ecfe2a9d8abf02d78b471d242f0a4a1fb9094dec1de2c240ba
//META: purpose=An unknown preprocessor directive renders E15101 with the directive name interpolated.
//META: intent=functional
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pin `unknown-preprocessor-directive` (15101) to its rendered
// code. `~directive` is the interpolated offending directive name.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):
#thisisbogus
/*CHECK:
 ^^^^^^^^^^^ E15101
 ^^^^^^^^^^^ unknown preprocessor directive 'thisisbogus'
*/
void main() {}
