//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T13:22:38+00:00
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: purpose=A reordering swizzle (`v.wzyx`) and a duplicating r-value swizzle (`v.xxy`) both produce the expected component permutation/duplication.
//META: intent=characterization
//META: covers=source/slang/slang-check-expr.cpp
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterizes the success path of CheckSwizzleExpr: element indices are
// taken from the component letters in order, so `v.wzyx` reverses the vector
// and `v.xxy` repeats the first component. The printed values prove the index
// mapping and that a duplicate component is allowed in an r-value position.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    float4 v = float4(1, 2, 3, 4);
    float4 r = v.wzyx;
    float3 d = v.xxy;
    // CHECK: 4.000000 3.000000 2.000000 1.000000 / 1.000000 1.000000 2.000000
    printf("%f %f %f %f / %f %f %f\n", r.x, r.y, r.z, r.w, d.x, d.y, d.z);
}
