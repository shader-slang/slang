//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#error-code-namespace
//META: doc_section_digest=e5426f9afa17becb9e7d33c23301014e082fad12bbf8aff1c4a169d90575556e
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
