//TEST:SIMPLE(filecheck=SCALAR): -target spirv -entry computeMain -stage compute -emit-spirv-directly -capability spvAtomicFloat16AddEXT
//DIAGNOSTIC_TEST:SIMPLE(diag=NO_FP16_ATOMIC,non-exhaustive): -target spirv -entry computeMain -stage compute -emit-spirv-directly -restrictive-capability-check -capability spirv_1_5
//DIAGNOSTIC_TEST:SIMPLE(diag=VECTOR_ONLY,non-exhaustive): -target spirv -entry computeMain -stage compute -emit-spirv-directly -restrictive-capability-check -capability spvAtomicFloat16VectorNV
//DIAGNOSTIC_TEST:SIMPLE(diag=EMULATED_NO_VECTOR,non-exhaustive): -target spirv -entry computeMain -stage compute -emit-spirv-directly -restrictive-capability-check -capability spvAtomicFloat16AddEXT -DTEST_EMULATED
//TEST:SIMPLE(filecheck=BOTH): -target spirv -entry computeMain -stage compute -emit-spirv-directly -capability spvAtomicFloat16AddEXT -capability spvAtomicFloat16VectorNV
//TEST:SIMPLE(filecheck=VECTOR): -target spirv -entry computeMain -stage compute -emit-spirv-directly -capability spvAtomicFloat16VectorNV -DTEST_DIRECT_VECTOR_ATOMIC
//TEST:SIMPLE(filecheck=EMULATED): -target spirv -entry computeMain -stage compute -emit-spirv-directly -capability spvAtomicFloat16VectorNV -DTEST_EMULATED
//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute -capability cuda_sm_7_0

RWByteAddressBuffer tmpBuffer;
RWStructuredBuffer<float> outputBuffer;
RWStructuredBuffer<half2> vectorBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
#ifdef TEST_DIRECT_VECTOR_ATOMIC
    __atomic_add(vectorBuffer[0], half2(1.0h, 2.0h));
#elif defined(TEST_EMULATED)
    half originalValue;
    tmpBuffer.InterlockedAddF16Emulated(0, 1.0h, originalValue);
// EMULATED_NO_VECTOR: entry point uses capabilities not in specified profile
// EMULATED_NO_VECTOR: Missing capabilities are: 'spvAtomicFloat16VectorNV'
    outputBuffer[0] = float(originalValue);
    tmpBuffer.InterlockedAddF16Emulated(2, 1.0h, originalValue);
    outputBuffer[1] = float(originalValue);
#else
    half originalValue;
    tmpBuffer.InterlockedAddF16(0, 1.0h, originalValue);
// NO_FP16_ATOMIC: entry point uses capabilities not in specified profile
// NO_FP16_ATOMIC: Missing capabilities are: 'spvAtomicFloat16AddEXT'
// VECTOR_ONLY: entry point uses capabilities not in specified profile
// VECTOR_ONLY: Missing capabilities are: 'spvAtomicFloat16AddEXT'
    outputBuffer[0] = float(originalValue);
    tmpBuffer.InterlockedAddF16(2, 1.0h, originalValue);
    outputBuffer[1] = float(originalValue);
    tmpBuffer.InterlockedAddF16(4, 1.0h, originalValue);
    outputBuffer[2] = float(originalValue);
    tmpBuffer.InterlockedAddF16(6, 1.0h, originalValue);
    outputBuffer[3] = float(originalValue);

    uint dynamicByteAddress = (dispatchThreadID.x & 1) * 2;
    tmpBuffer.InterlockedAddF16(dynamicByteAddress, 1.0h, originalValue);
    outputBuffer[4] = float(originalValue);
#endif
}

// SCALAR-NOT: OpCapability AtomicFloat16VectorNV
// SCALAR-NOT: OpExtension "SPV_NV_shader_atomic_fp16_vector"
// SCALAR: OpCapability AtomicFloat16AddEXT
// SCALAR: OpExtension "SPV_EXT_shader_atomic_float16_add"
// SCALAR-NOT: OpAtomicFAddEXT %v2half
// SCALAR-COUNT-5: OpAtomicFAddEXT %half
// SCALAR-NOT: OpAtomicFAddEXT %v2half
// SCALAR-NOT: OpCapability AtomicFloat16VectorNV
// SCALAR-NOT: OpExtension "SPV_NV_shader_atomic_fp16_vector"

// BOTH-NOT: OpCapability AtomicFloat16VectorNV
// BOTH-NOT: OpExtension "SPV_NV_shader_atomic_fp16_vector"
// BOTH: OpCapability AtomicFloat16AddEXT
// BOTH: OpExtension "SPV_EXT_shader_atomic_float16_add"
// BOTH-NOT: OpAtomicFAddEXT %v2half
// BOTH-COUNT-5: OpAtomicFAddEXT %half
// BOTH-NOT: OpAtomicFAddEXT %v2half
// BOTH-NOT: OpAtomicCompareExchange
// BOTH-NOT: OpCapability AtomicFloat16VectorNV
// BOTH-NOT: OpExtension "SPV_NV_shader_atomic_fp16_vector"

// VECTOR-NOT: OpCapability AtomicFloat16AddEXT
// VECTOR-NOT: OpExtension "SPV_EXT_shader_atomic_float16_add"
// VECTOR: OpCapability AtomicFloat16VectorNV
// VECTOR: OpExtension "SPV_NV_shader_atomic_fp16_vector"

// EMULATED-NOT: OpCapability AtomicFloat16AddEXT
// EMULATED-NOT: OpExtension "SPV_EXT_shader_atomic_float16_add"
// EMULATED: OpCapability AtomicFloat16VectorNV
// EMULATED: OpExtension "SPV_NV_shader_atomic_fp16_vector"
// EMULATED-COUNT-2: OpAtomicFAddEXT
// EMULATED-DAG: [[LOW:%[0-9]+]] = OpConstantComposite %v2half %half_0x1p_0 %half_0x0p_0
// EMULATED-DAG: [[HIGH:%[0-9]+]] = OpConstantComposite %v2half %half_0x0p_0 %half_0x1p_0
// EMULATED: [[LOW_ATOMIC:%[0-9]+]] = OpAtomicFAddEXT %v2half {{%[0-9]+}} %uint_1 %uint_0 [[LOW]]
// EMULATED: OpCompositeExtract %half [[LOW_ATOMIC]] 0
// EMULATED: [[HIGH_ATOMIC:%[0-9]+]] = OpAtomicFAddEXT %v2half {{%[0-9]+}} %uint_1 %uint_0 [[HIGH]]
// EMULATED: OpCompositeExtract %half [[HIGH_ATOMIC]] 1

// CUDA-COUNT-5: atomicAdd(
