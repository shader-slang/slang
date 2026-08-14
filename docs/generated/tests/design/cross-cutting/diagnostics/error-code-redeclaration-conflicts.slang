//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#error-code-namespace
//META: doc_section_digest=e5426f9afa17becb9e7d33c23301014e082fad12bbf8aff1c4a169d90575556e
//META: purpose=A conflicting redeclaration renders E30200 together with its companion `see previous declaration` note.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pin `redeclaration` (30200) to its rendered code. Conflict on
// variable types: declare `x` as int, then float — the diagnostic
// names the offending identifier and the chain has a "see previous
// declaration" companion note.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):
int x;
/*CHECK:
    ^ see previous declaration of 'x'
*/
float x;
/*CHECK:
      ^ E30200
      ^ declaration of 'x' conflicts with existing declaration
*/

void main() {}
