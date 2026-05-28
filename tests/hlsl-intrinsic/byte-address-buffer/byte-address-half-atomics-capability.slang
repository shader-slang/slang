//TEST:SIMPLE(filecheck=SCALAR): -target spirv -entry computeMain -stage compute -emit-spirv-directly -capability spvAtomicFloat16AddEXT
//TEST:SIMPLE(filecheck=FALLBACK): -target spirv -entry computeMain -stage compute -emit-spirv-directly -capability spvAtomicFloat16VectorNV
//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute -capability cuda_sm_7_0

RWByteAddressBuffer tmpBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    half originalValue;
    tmpBuffer.InterlockedAddF16(0, 1.0h, originalValue);
    tmpBuffer.InterlockedAddF16(2, 1.0h, originalValue);
}

// SCALAR-NOT: OpCapability AtomicFloat16VectorNV
// SCALAR-NOT: OpExtension "SPV_NV_shader_atomic_fp16_vector"
// SCALAR: OpCapability AtomicFloat16AddEXT
// SCALAR: OpExtension "SPV_EXT_shader_atomic_float16_add"
// SCALAR-NOT: OpCapability AtomicFloat16VectorNV
// SCALAR-NOT: OpExtension "SPV_NV_shader_atomic_fp16_vector"

// FALLBACK-NOT: OpCapability AtomicFloat16AddEXT
// FALLBACK-NOT: OpExtension "SPV_EXT_shader_atomic_float16_add"
// FALLBACK-NOT: OpCapability AtomicFloat16VectorNV
// FALLBACK-NOT: OpExtension "SPV_NV_shader_atomic_fp16_vector"
// FALLBACK-NOT: OpAtomicFAddEXT
// FALLBACK: OpAtomicIAdd
// FALLBACK: OpAtomicCompareExchange
// FALLBACK-NOT: OpAtomicFAddEXT

// CUDA: atomicAdd(
// CUDA: _getPtrAt<half>(
