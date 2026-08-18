//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T13:22:38+00:00
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: purpose=Subscripting a scalar value (`x[0]` where x is a float) is rejected because no subscript declarations exist for the type (E30013).
//META: intent=characterization
//META: covers=source/slang/slang-check-expr.cpp
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterizes the IndexExpr check: a base type with no `__subscript`
// declarations (a plain scalar `float`) yields SubscriptNonArray (E30013)
// "no subscript declarations found for type 'float'".

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry test -stage fragment

void test() { float x = 1.0; float y = x[0]; }
//CHECK:                                ^ invalid subscript expression
//CHECK:                                ^ no subscript declarations found for type 'float'

// CHECK: E30013
