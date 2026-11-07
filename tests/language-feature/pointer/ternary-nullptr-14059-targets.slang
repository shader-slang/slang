// The reporter's shape for https://github.com/shader-slang/slang/issues/14059:
// `cond ? ptr : nullptr`, in either operand order, compiles for Metal, CUDA and
// SPIR-V, with each null arm assigning a null pointer.

//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry computeMain -stage compute

//METAL: n0_{{[0-9]+}} = nullptr;
//METAL: n1_{{[0-9]+}} = nullptr;
//CUDA: n0_{{[0-9]+}} = nullptr;
//CUDA: n1_{{[0-9]+}} = nullptr;
//SPIRV: %[[ZERO:[a-zA-Z0-9_]+]] = OpConstant %ulong 0
//SPIRV: OpConvertUToPtr %_ptr_PhysicalStorageBuffer_v3float %[[ZERO]]
//SPIRV: OpConvertUToPtr %_ptr_PhysicalStorageBuffer_v3float %[[ZERO]]

[numthreads(64, 1, 1)]
void computeMain(
    uniform float3* normals,
    uniform int* index,
    uniform float* outv,
    uniform int useIt,
    uint3 tid: SV_DispatchThreadID)
{
    float3* n0 = (useIt != 0) ? normals + index[tid.x] : nullptr;
    float3* n1 = (useIt != 0) ? nullptr : normals + index[tid.x];
    outv[tid.x] = ((n0 != nullptr) ? n0[0].x : -1.0f) + ((n1 != nullptr) ? n1[0].x : -1.0f);
}
