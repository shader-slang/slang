//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#anatomy-of-a-diagnostic-entry
//META: doc_section_digest=3a623795b5dd78e308025c61705ad0257495b87ee059b06c6f6541a5d86c1a61
//META: purpose=The doc's worked entry `function-redeclaration-with-different-return-type` renders code 30202 with its `~decl`, `~newReturnType:Type` and `~prevReturnType:Type` parameters interpolated.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the doc's worked entry end-to-end: redeclaring a function with a
// different return type fires the entry declared as (name, code 30202, title,
// primary span). CHECK pins the rendered code E30202 and the span text with
// the `~decl` name plus both `~...ReturnType:Type` parameters substituted, so
// the test fails if any of the declared pieces stops reaching the user. The
// companion note on the first declaration is annotated in place to keep the
// exhaustive diagnostic check satisfied.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

int getValue(int x);
/*CHECK:
    ^^^^^^^^ see previous declaration of 'getValue'
*/

float getValue(int x);
//CHECK: E30202
//CHECK: function 'getValue' declared to return 'float' was previously declared to return 'int'
