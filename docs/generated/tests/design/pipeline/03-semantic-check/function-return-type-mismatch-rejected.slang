//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#failure-modes
//META: doc_section_digest=8518a72edfb4aa449935321fb64c48a945528cb8e3f926bdb661166c52241bd1
//META: purpose=Declaring the same function name twice with different return types is rejected with the function-return-type-mismatch diagnostic (E30202).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary of function-redefinition coverage: the doc says all
// semantic errors flow through DiagnosticSink; here the offending
// shape is two declarations of the same name disagreeing on return
// type (E30202), distinct from the body-redefinition E30201.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

int f(int x);
float f(int x);
/*CHECK:
      ^ function return type mismatch
      ^ function 'f' declared to return 'float' was previously declared to return 'int'
*/
// CHECK: E30202

void main() {}
