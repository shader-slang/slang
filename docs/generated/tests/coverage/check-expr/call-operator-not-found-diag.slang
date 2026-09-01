//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T13:22:38+00:00
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: purpose=Calling a struct value that has no call operator (`s(3)`) is rejected with "no call operation found" (E30016).
//META: intent=characterization
//META: covers=source/slang/slang-check-expr.cpp
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterizes the CheckInvokeExprWithCheckedOperands fallback: when the
// callee is a value whose type defines no call operator, the check emits
// CallOperatorNotFound (E30016) naming the offending type.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry test -stage fragment

struct S { int v; }
void test() { S s; s(3); }
//CHECK:            ^ no call operation found for type
//CHECK:            ^ no call operation found for type 'S'

// CHECK: E30016
