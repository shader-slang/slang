//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#preprocessor-directives
//META: doc_section_digest=c3ad01b7a04e24b43f66e2d017e85a491e5a0fbbf99a28a1c828245742a23613
//META: purpose=Boundary probe of `#error`: a very long message body (upper edge of the message axis) is preserved verbatim in the diagnostic.
//META: intent=boundary
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `#error` with a long message body. The directive's text is
// echoed verbatim into the diagnostic, so we FileCheck a unique
// substring near the end to confirm the long message survives.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

#error this is a very long preprocessor error message designed to probe the upper boundary of the message text and contain a distinctive sentinel SENTINEL_END
//CHECK: SENTINEL_END
//CHECK: E15900

void main() {}
