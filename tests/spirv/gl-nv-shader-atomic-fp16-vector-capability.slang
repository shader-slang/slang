//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry computeMain -stage compute -emit-spirv-directly

[require(GL_NV_shader_atomic_fp16_vector)]
[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
}

// CHECK-NOT: OpCapability AtomicFloat16VectorNV
// CHECK-NOT: OpExtension "SPV_NV_shader_atomic_fp16_vector"
// CHECK: OpMemoryModel
