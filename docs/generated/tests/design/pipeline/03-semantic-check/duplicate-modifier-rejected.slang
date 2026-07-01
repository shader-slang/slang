//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=273b0c1d7bd60576ed4d24081c0d1b9f61952281
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#modifier-validation
//META: doc_section_digest=9163d71b0ed4735c5c28e2877570515476519666c5a6da87256143a8fab12f94
//META: purpose=Modifier validation rejects a redundant repetition of the same modifier (E31202) on one declaration.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Modifier validation rejects duplicated modifiers, naming the
// duplicate and the existing modifier.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

static static const int X = 5;
/*CHECK:
       ^^^^^^ duplicate modifier
       ^^^^^^ modifier 'static' is redundant or conflicting with existing modifier 'static'
*/
// CHECK: E31202

void main() { int y = X; }
