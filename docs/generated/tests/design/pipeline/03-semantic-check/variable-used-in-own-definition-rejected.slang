//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#failure-modes
//META: doc_section_digest=8518a72edfb4aa449935321fb64c48a945528cb8e3f926bdb661166c52241bd1
//META: purpose=Decl checking rejects a variable whose initializer depends on the variable itself ("used in its own definition").
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Cyclic-reference boundary at the value level: a `static const`
// whose initializer references itself is rejected.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

static const int X = X + 1;
//CHECK:         ^ the initial-value expression for variable 'X' depends on the value of the variable itself

void main() { int y = X; }
