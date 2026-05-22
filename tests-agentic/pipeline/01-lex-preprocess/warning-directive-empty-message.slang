//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/llm-generated/pipeline/01-lex-preprocess.md#preprocessor-directives
//META: doc_section_digest=3d8d27a59aa70cbff18bfb217be0513b6ff9ff565fb5ebcb7298879f0a47f2fd
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
