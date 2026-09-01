//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T13:22:38+00:00
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: purpose=Using the comma operator inside an expression emits a may-be-unintended warning (E41024) but still compiles.
//META: intent=characterization
//META: covers=source/slang/slang-check-expr.cpp
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterizes the InfixExpr comma-operator branch in checkExpr: a top-level
// comma sequence in a parenthesized expression triggers the
// CommaOperatorUsedInExpression warning (E41024); CHECK pins the warning text.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry test -stage fragment

int test(int a, int b) { return (a, b); }
//CHECK:                          ^ comma operator used in expression
//CHECK:                          ^ comma operator used in expression (may be unintended)

// CHECK: E41024
