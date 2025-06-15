//TEST:SIMPLE(filecheck=CHECK):-target spirv -emit-spirv-directly -incomplete-library -entry computeMain -stage compute -O0

// CHECK: OpCapability Linkage
// CHECK: OpDecorate %f LinkageAttributes "_S1fp2pi_hi_hh" Import
// CHECK: %[[EXP_FUNC_TYPE:[A-Za-z0-9_]+]] = OpTypeFunction %half %half %half
// CHECK: %f = OpFunction %half None %[[EXP_FUNC_TYPE]]
extern float16_t f(float16_t x, float16_t y);

RWStructuredBuffer<float16_t> buff;

[numthreads(1,1,1)]
void computeMain()
{
    buff[2] = f(buff[0], buff[1]);
}
