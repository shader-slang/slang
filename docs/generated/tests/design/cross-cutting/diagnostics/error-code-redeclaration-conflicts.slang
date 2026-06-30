//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=d245d3be9942f0847c45c138b4c2780e7df19ecb
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#error-codes-and-the-name-field
//META: doc_section_digest=58de6e7299bd58eba9a686426fc010a16e2745f7e3231042be33437e83834630
//META: purpose=Negative: a conflicting variable redeclaration renders E30200 with the documented "declaration conflicts with existing declaration" message and emits a companion "see previous declaration" note.
//META: intent=negative
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
