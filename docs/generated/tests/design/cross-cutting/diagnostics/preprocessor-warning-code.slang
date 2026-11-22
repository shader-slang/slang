//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:32:51Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#error-code-namespace
//META: doc_section_digest=5ef206950ef0e6ecfe2a9d8abf02d78b471d242f0a4a1fb9094dec1de2c240ba
//META: purpose=Preprocessor warnings take ids from the conventional 15xxx subsystem range (E15205).
//META: intent=functional
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that preprocessor-stage warnings take ids from the conventional
// 15xxx subsystem range. An unset macro is used in an `#if` condition; the
// caret-anchored CHECK pins code 15205 and the message naming the macro.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

#if UNSET_MACRO
/*CHECK:
    ^^^^^^^^^^^ E15205
    ^^^^^^^^^^^ undefined identifier 'UNSET_MACRO' in preprocessor expression will evaluate to zero
*/
#endif

void main() {}
