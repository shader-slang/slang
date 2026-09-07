//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T13:22:38+00:00
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: purpose=A multi-component matrix swizzle (`m._m00_m11`) lowers to a vector built from per-element matrix subscripts.
//META: intent=characterization
//META: covers=source/slang/slang-check-expr.cpp
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterizes CheckMatrixSwizzleExpr's zero-indexed `_mRC` parsing and the
// emit of a multi-element matrix swizzle: `_m00_m11` becomes a float2 whose
// elements are the (0,0) and (1,1) entries, each emitted as a [row][col]
// matrix subscript. FileCheck (CI) pins the constructed vector form.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry test -stage fragment

float2 test(float2x2 m) { return m._m00_m11; }
// CHECK: return float2(m_{{[0-9]+}}[int(0)][int(0)], m_{{[0-9]+}}[int(1)][int(1)]);
