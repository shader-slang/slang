//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#preprocessor-directives
//META: doc_section_digest=a79d944b7d2a39d014afe31606a0e3f7d96f6912c534eadd8d9b1160507f3183
//META: purpose=Boundary probe of `#warning`: a bare `#warning` with no message text still fires the documented preprocessor-warning diagnostic.
//META: intent=boundary
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `#warning` with the empty message — the lower edge of the
// message axis. The warning diagnostic must still be emitted with
// nothing after `#warning:`.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

#warning
/*CHECK:
 ^^^^^^^ preprocessor warning
 ^^^^^^^ #warning:
*/
// CHECK: W15901

void main() {}
