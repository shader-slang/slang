//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:32:51Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#anatomy-of-a-diagnostic-entry
//META: doc_section_digest=f22b494b8e869f4fe09a0366d2108166de2b12d27d47542c79c644cf98a38302
//META: purpose=A `warning` entry carrying a note renders the warning span plus its companion note at the previous definition.
//META: intent=functional
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a `warning` entry carrying a note renders both records. A
// macro is defined twice; the annotations pin the warning (code 15400) at the
// second definition and the companion note at the first, so losing either
// record fails the exhaustive check.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,exit-code=0):

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
