//TEST:SIMPLE(filecheck=NEGATIVE): -target spirv -entry computeMain -stage compute -emit-spirv-directly
//TEST:SIMPLE(filecheck=POSITIVE): -target spirv -entry computeMain -stage compute -emit-spirv-directly -DPOSITIVE_VECTOR_ATOMIC

// Requiring the GLSL extension should preserve the old SPIR-V 1.0 compatibility path and
// should not declare the NV vector capability until a vector atomic operation needs it.
RWStructuredBuffer<half2> vectorBuffer;

[require(GL_NV_shader_atomic_fp16_vector)]
[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
#ifdef POSITIVE_VECTOR_ATOMIC
    __atomic_add(vectorBuffer[0], half2(1.0h, 2.0h));
#endif
}

// NEGATIVE-NOT: OpCapability AtomicFloat16VectorNV
// NEGATIVE-NOT: OpExtension "SPV_NV_shader_atomic_fp16_vector"
// NEGATIVE: OpMemoryModel

// POSITIVE: OpCapability AtomicFloat16VectorNV
// POSITIVE: OpExtension "SPV_NV_shader_atomic_fp16_vector"
