//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T13:22:38+00:00
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: purpose=A swizzle component beyond the vector's element count (`v.z` on a `float2`) is rejected as a missing member (E30027).
//META: intent=characterization
//META: covers=source/slang/slang-check-expr.cpp
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterizes CheckSwizzleExpr's out-of-range branch: 'z' maps to element
// index 2, which is >= the float2 element count, so the swizzle check returns
// nullptr and member lookup ultimately reports E30027 "member not found".

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry test -stage fragment

float test(float2 v) { return v.z; }
//CHECK:                        ^ member not found
//CHECK:                        ^ 'z' is not a member of 'vector<float,2>'.

// CHECK: E30027
