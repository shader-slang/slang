//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#modifier-validation
//META: doc_section_digest=6165fbd3f46bb23f31d4481667f01c7f841053292ade0127deceffcbed0876dc
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
