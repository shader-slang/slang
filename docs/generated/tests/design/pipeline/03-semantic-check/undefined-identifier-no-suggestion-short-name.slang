//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#failure-modes
//META: doc_section_digest=9aab17755318b3eba1171ef991b0e4f492c51e9b08946482351cebd403903cdb
//META: purpose=No suggestion is made for a name under three characters, so a two-character misspelling of a two-character name gets no suggestion.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the conservatism boundary of the suggestion. `ac` is one edit from
// the in-scope `ab`, but the name is below the three-character
// minimum, so no suggestion is made; an unscaled rule would emit a suggestion
// here. CHECK pins the undefined-identifier error and its message, and
// exhaustive mode turns "nothing else was reported" — in particular no
// suggestion note — into part of the assertion.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    int ab = 1;
    int y = ac;
//CHECK: E30015
//CHECK: undefined identifier 'ac'.
}
