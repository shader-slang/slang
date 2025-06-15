//TEST:SIMPLE(filecheck=CHECK):-target spirv -emit-spirv-directly -incomplete-library -O0

// CHECK: OpCapability Linkage

// CHECK-NOT: OpDecorate %g LinkageAttributes
float16_t g(float16_t x)
{
    return x + float16_t(1.0);
}

// CHECK: OpDecorate %f LinkageAttributes "_S1fp2pi_hi_hh" Export
// CHECK: %[[EXP_FUNC_TYPE:[A-Za-z0-9_]+]] = OpTypeFunction %half %half %half
// CHECK: %f = OpFunction %half None %[[EXP_FUNC_TYPE]]
export float16_t f(float16_t x, float16_t y)
{
    return x + g(y);
}
