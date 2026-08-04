//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=d245d3be9942f0847c45c138b4c2780e7df19ecb
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#error-codes-and-the-name-field
//META: doc_section_digest=58de6e7299bd58eba9a686426fc010a16e2745f7e3231042be33437e83834630
//META: purpose=Negative: an unknown preprocessor directive renders E15101 with the directive name interpolated into the message.
//META: intent=negative
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
