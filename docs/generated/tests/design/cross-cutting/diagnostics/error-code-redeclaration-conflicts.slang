//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:32:51Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#error-code-namespace
//META: doc_section_digest=5ef206950ef0e6ecfe2a9d8abf02d78b471d242f0a4a1fb9094dec1de2c240ba
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
