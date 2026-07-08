//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:00:00Z
//META: source_commit=8ac9e49a5b7020af6a8040544b9f76f0fbf40898
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#anatomy-of-a-legacy-diagnostic
//META: doc_section_digest=0f16cae8d67a6c5de089898c5da553b4a34d61f8d57f24ac46fb78de93805794
//META: purpose=The legacy worked example function-redeclaration-with-different-return-type renders E30202 with the function name and both return types interpolated via its ~name:Type span parameters.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the doc's legacy-diagnostic worked example is live end-to-end:
// redeclaring a function with a different return type fires the diagnostic
// declared as (name, code 30202, title, primary span). The CHECK lines pin
// the rendered code E30202 and the span message with the call-site-supplied
// ~decl name and ~newReturnType/~prevReturnType typed parameters filled in
// as quoted type names, proving the declaration form reaches the user.
// The companion "see previous declaration" note on the first declaration
// is annotated in place so the exhaustive diagnostic check stays intact.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

int getValue(int x);
/*CHECK:
    ^^^^^^^^ see previous declaration of 'getValue'
*/

float getValue(int x);
//CHECK: E30202
//CHECK: function 'getValue' declared to return 'float' was previously declared to return 'int'
