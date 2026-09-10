//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T13:22:38+00:00
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: purpose=Two overloads that are each a best match on one argument but not the other produce an ambiguous-call error (E39999) with per-candidate notes (E40011).
//META: intent=characterization
//META: covers=source/slang/slang-check-expr.cpp
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterizes the overload-resolution tie-break failure path: `f(1, 1)` has
// two candidates neither of which strictly dominates, so resolution reports an
// ambiguous call (E39999) and lists each rejected candidate as a note (E40011).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry test -stage fragment

int f(int a, float b) { return a; }
int f(float a, int b) { return int(a); }
void test() { f(1, 1); }
//CHECK:       ^ ambiguous call to 'f' with arguments of type (int, int)

// CHECK: E39999
// CHECK: E40011
