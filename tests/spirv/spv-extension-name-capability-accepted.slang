// A capability may still be spelled as the SPIR-V extension name, `SPV_*`, for backward
// compatibility. The `spv<Feature>` spelling is the one users are meant to write and is the one
// listed by `-h`, but the `SPV_*` spelling is accepted silently and behaves identically.
//
// `SPV_KHR_cooperative_matrix` inherits a SPIR-V 1.6 floor and the Vulkan memory model, so both
// spellings must produce the same module header and declare the same capability.

//TEST:SIMPLE(filecheck=EXT): -target spirv-asm -emit-spirv-directly -capability SPV_KHR_cooperative_matrix -entry main -stage compute
//TEST:SIMPLE(filecheck=EXT): -target spirv-asm -emit-spirv-directly -profile spirv_1_6+SPV_KHR_cooperative_matrix -entry main -stage compute
//TEST:SIMPLE(filecheck=EXT): -target spirv-asm -emit-spirv-directly -capability spvCooperativeMatrixKHR -entry main -stage compute

//EXT-NOT: warning
//EXT-DAG: ; Version: 1.6
//EXT-DAG: OpCapability VulkanMemoryModel

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = 5;
}
