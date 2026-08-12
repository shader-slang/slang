//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#anatomy-of-a-diagnostic-entry
//META: doc_section_digest=79bb0fc1ac8583fef268d4e89fe89d0e6aa8ce66124dff7c481098ca077adbae
//META: purpose=A `warning` entry carrying a note renders the warning span plus its companion note at the previous definition.
//META: intent=functional
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a `warning` entry carrying a note renders both records. A
// macro is defined twice; the annotations pin the warning (code 15400) at the
// second definition and the companion note at the first, so losing either
// record fails the exhaustive check.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

#define MY_MACRO 1
/*CHECK:
        ^^^^^^^^ see previous definition of 'MY_MACRO'
*/

#define MY_MACRO 2
/*CHECK:
        ^^^^^^^^ E15400
        ^^^^^^^^ redefinition of macro 'MY_MACRO'
*/

void main() {}
