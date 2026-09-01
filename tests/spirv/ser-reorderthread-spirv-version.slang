// Test for issue #12097: the physical-storage-buffer dependency of the SER extensions at
// SPIR-V 1.4 must be added for EVERY path that requires an SER capability, not only the
// HitObject type/decoration emit. ReorderThread() lowers via a `spirv_asm` block in
// hlsl.meta.slang that declares the SER OpExtension/OpCapability directly; because that
// block still funnels through requireSPIRVCapability, it gains the same 1.4 dependency.

//TEST:SIMPLE(filecheck=NV14): -target spirv-asm -emit-spirv-directly -profile spirv_1_4 -capability spvShaderInvocationReorderNV -entry main -stage raygeneration
//TEST:SIMPLE(filecheck=EXT14): -target spirv-asm -emit-spirv-directly -profile spirv_1_4 -capability spvShaderInvocationReorderEXT -entry main -stage raygeneration

// Higher profiles keep their core version with no spurious PSB extension (core from 1.5).
//TEST:SIMPLE(filecheck=NV15): -target spirv-asm -emit-spirv-directly -profile spirv_1_5 -capability spvShaderInvocationReorderNV -entry main -stage raygeneration
//TEST:SIMPLE(filecheck=EXT15): -target spirv-asm -emit-spirv-directly -profile spirv_1_5 -capability spvShaderInvocationReorderEXT -entry main -stage raygeneration

// Binary modules so CI's SLANG_RUN_SPIRV_VALIDATION=1 runs spirv-val on the real 1.4 module.
//TEST:SIMPLE(filecheck=NV14): -target spirv -emit-spirv-directly -profile spirv_1_4 -capability spvShaderInvocationReorderNV -entry main -stage raygeneration
//TEST:SIMPLE(filecheck=EXT14): -target spirv -emit-spirv-directly -profile spirv_1_4 -capability spvShaderInvocationReorderEXT -entry main -stage raygeneration

// NV14: Version: 1.4
// NV14-DAG: OpCapability ShaderInvocationReorderNV
// NV14-DAG: OpExtension "SPV_KHR_physical_storage_buffer"
// NV14-DAG: OpExtension "SPV_NV_shader_invocation_reorder"
// NV14-DAG: OpMemoryModel Logical GLSL450

// EXT14: Version: 1.4
// EXT14-DAG: OpCapability ShaderInvocationReorderEXT
// EXT14-DAG: OpExtension "SPV_KHR_physical_storage_buffer"
// EXT14-DAG: OpExtension "SPV_EXT_shader_invocation_reorder"
// EXT14-DAG: OpMemoryModel Logical GLSL450

// NV15: Version: 1.5
// NV15-NOT: OpExtension "SPV_KHR_physical_storage_buffer"

// EXT15: Version: 1.5
// EXT15-NOT: OpExtension "SPV_KHR_physical_storage_buffer"

[shader("raygeneration")]
void main()
{
    ReorderThread(1u, 1u);
}
