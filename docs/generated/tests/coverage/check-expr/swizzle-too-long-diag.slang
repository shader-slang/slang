//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T13:22:38+00:00
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: purpose=A swizzle longer than four components (`v.xxxxx`) is rejected as a missing member (E30027).
//META: intent=characterization
//META: covers=source/slang/slang-check-expr.cpp
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterizes CheckSwizzleExpr's `elementIndices.getCount() >= 4` guard:
// a fifth valid component sets anyError and the check returns nullptr, so the
// member lookup reports E30027 rather than a swizzle-specific diagnostic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry test -stage fragment

float test(float4 v) { return v.xxxxx.x; }
//CHECK:                        ^^^^^ member not found
//CHECK:                        ^^^^^ 'xxxxx' is not a member of 'vector<float,4>'.

// CHECK: E30027
