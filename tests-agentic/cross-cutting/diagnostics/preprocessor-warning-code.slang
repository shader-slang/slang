//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T12:30:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/llm-generated/cross-cutting/diagnostics.md#error-codes-and-the-name-field
//META: doc_section_digest=58de6e7299bd58eba9a686426fc010a16e2745f7e3231042be33437e83834630
//META: purpose=Preprocessor warnings carry integer ids in the same space as semantic warnings; undefined identifier in `#if` is 15205.
//META: intent=negative
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says every diagnostic has a unique integer id, independent
// of pipeline stage. Verify that a preprocessor-stage warning carries
// its code (15205, undefined-identifier-in-preprocessor-expression)
// just like checker warnings do.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

#if UNSET_MACRO
/*CHECK:
    ^^^^^^^^^^^ 15205
    ^^^^^^^^^^^ undefined identifier 'UNSET_MACRO' in preprocessor expression will evaluate to zero
*/
#endif

void main() {}
