//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=273b0c1d7bd60576ed4d24081c0d1b9f61952281
//META: doc_ref=docs/llm-generated/pipeline/03-semantic-check.md#failure-modes
//META: doc_section_digest=ba8996c1b140750bb2e807ee33d646c0a62d626b5b41e86bf40fe00640f5b270
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
