//TEST:SIMPLE(filecheck=SCALAR): -target spirv -entry computeMain -stage compute -emit-spirv-directly -skip-spirv-validation -ignore-capabilities -capability spvAtomicFloat16AddEXT
//TEST:SIMPLE(filecheck=VECTOR): -target spirv -entry computeMain -stage compute -emit-spirv-directly -skip-spirv-validation -ignore-capabilities -capability GL_NV_shader_atomic_fp16_vector

RWByteAddressBuffer tmpBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    half originalValue;
    tmpBuffer.InterlockedAddF16(2, 1.0h, originalValue);
}

// SCALAR: OpCapability AtomicFloat16AddEXT
// SCALAR: OpExtension "SPV_EXT_shader_atomic_float16_add"
// SCALAR-NOT: OpCapability AtomicFloat16VectorNV

// VECTOR: OpCapability AtomicFloat16VectorNV
// VECTOR: OpExtension "SPV_NV_shader_atomic_fp16_vector"
// VECTOR-NOT: OpCapability AtomicFloat16AddEXT
