//TEST:SIMPLE_EX(filecheck=SPIRV): tests/bugs/13306/specialization-constant-shadow.hlsl -Gec -target spirv -entry main -validate-ir
//TEST:SIMPLE_EX(filecheck=SPIRV): tests/bugs/13306/specialization-constant-shadow.hlsl -target spirv -entry main -validate-ir
//TEST:SIMPLE_EX(filecheck=WRITE): tests/bugs/13306/specialization-constant-shadow.hlsl -Gec -target spirv -entry main -no-codegen -DTRY_WRITE

// A specialization constant without a written `const` still identifies the parameter in
// `numthreads` and arithmetic. Compatibility must preserve that identity and reject writes.
[[vk::constant_id(7)]] uint specialization = 3;
RWStructuredBuffer<uint> output;

// SPIRV-DAG: OpExecutionModeId %{{[0-9A-Za-z_]+}} LocalSizeId %[[SPECIALIZATION:[0-9A-Za-z_]+]] %{{[0-9A-Za-z_]+}} %{{[0-9A-Za-z_]+}}
// SPIRV-DAG: OpDecorate %[[SPECIALIZATION]] SpecId 7
// SPIRV-DAG: %[[SPECIALIZATION]] = OpSpecConstant %uint 3
// SPIRV-DAG: %[[COMPUTED:[0-9A-Za-z_]+]] = OpSpecConstantOp %uint IAdd %[[SPECIALIZATION]] %{{[0-9A-Za-z_]+}}
// SPIRV: OpStore %{{[0-9A-Za-z_]+}} %[[COMPUTED]]

[shader("compute")]
#ifdef TRY_WRITE
[numthreads(1, 1, 1)]
#else
[numthreads(specialization, 1, 1)]
#endif
void main(uint3 tid : SV_DispatchThreadID)
{
#ifdef TRY_WRITE
    specialization = 4;
    // WRITE: error[E30011]: left of '=' is not an l-value
    // WRITE-NOT: error[
#endif
    output[tid.x] = specialization + 1;
}
