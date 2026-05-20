//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=d245d3be9942f0847c45c138b4c2780e7df19ecb
//META: doc_ref=docs/llm-generated/cross-cutting/diagnostics.md#error-codes-and-the-name-field
//META: doc_section_digest=58de6e7299bd58eba9a686426fc010a16e2745f7e3231042be33437e83834630
//META: purpose=Negative: a `#warning` preprocessor directive emits E15901 with the documented "preprocessor warning" code and the user message interpolated.
//META: intent=negative
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
