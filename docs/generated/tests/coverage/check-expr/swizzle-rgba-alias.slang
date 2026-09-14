//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T13:22:38+00:00
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: purpose=The color-channel swizzle letters r/g/b/a select the same components as x/y/z/w.
//META: intent=characterization
//META: covers=source/slang/slang-check-expr.cpp
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterizes the r/g/b/a -> 0/1/2/3 letter mapping in CheckSwizzleExpr.
// `c.r c.g c.b c.a` must print the same values that `c.x c.y c.z c.w` would,
// confirming the alias letters resolve to the identical element indices.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    float4 c = float4(10, 20, 30, 40);
    // CHECK: 10.000000 20.000000 30.000000 40.000000
    printf("%f %f %f %f\n", c.r, c.g, c.b, c.a);
}
