// Scalar fp16 add checks. `NO_FP16_ATOMIC` has no fp16 atomic capability; `VECTOR_ONLY`
// has the vector capability but intentionally lacks the scalar spvAtomicFloat16AddEXT.
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=RUNTIME_SCALAR):-vk -compute -entry computeMain -emit-spirv-directly -capability spvAtomicFloat16AddEXT -output-using-type -xslang -DTEST_RUNTIME_SCALAR
//TEST:SIMPLE(filecheck=SCALAR): -target spirv -entry computeMain -stage compute -emit-spirv-directly -capability spvAtomicFloat16AddEXT
//DIAGNOSTIC_TEST:SIMPLE(diag=NO_FP16_ATOMIC,non-exhaustive): -target spirv -entry computeMain -stage compute -emit-spirv-directly -restrictive-capability-check -capability spirv_1_5
//DIAGNOSTIC_TEST:SIMPLE(diag=VECTOR_ONLY,non-exhaustive): -target spirv -entry computeMain -stage compute -emit-spirv-directly -restrictive-capability-check -capability spvAtomicFloat16VectorNV

// Direct half-vector `__atomic_*` checks. `DIRECT` selects direct structured-buffer
// atomics; `NO_VECTOR` means only the scalar fp16 atomic capability is enabled.
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=RUNTIME_DIRECT):-vk -compute -entry computeMain -emit-spirv-directly -capability spvAtomicFloat16VectorNV -output-using-type -xslang -DTEST_RUNTIME_DIRECT
//DIAGNOSTIC_TEST:SIMPLE(diag=DIRECT_NO_VECTOR,non-exhaustive): -target spirv -entry computeMain -stage compute -emit-spirv-directly -restrictive-capability-check -capability spvAtomicFloat16AddEXT -DTEST_DIRECT_VECTOR_ATOMIC
//DIAGNOSTIC_TEST:SIMPLE(diag=DIRECT_VECTOR4_NO_VECTOR,non-exhaustive): -target spirv -entry computeMain -stage compute -emit-spirv-directly -restrictive-capability-check -capability spvAtomicFloat16AddEXT -DTEST_DIRECT_VECTOR4_ATOMIC
//DIAGNOSTIC_TEST:SIMPLE(diag=DIRECT_MIN_MAX_NO_VECTOR,non-exhaustive): -target spirv -entry computeMain -stage compute -emit-spirv-directly -restrictive-capability-check -capability spvAtomicFloat16AddEXT -DTEST_DIRECT_VECTOR_MIN_MAX
//DIAGNOSTIC_TEST:SIMPLE(diag=DIRECT_EXCHANGE_NO_VECTOR,non-exhaustive): -target spirv -entry computeMain -stage compute -emit-spirv-directly -restrictive-capability-check -capability spvAtomicFloat16AddEXT -DTEST_DIRECT_VECTOR_EXCHANGE
//DIAGNOSTIC_TEST:SIMPLE(diag=UNSUPPORTED_VECTOR_WIDTH,non-exhaustive): -target spirv -entry computeMain -stage compute -emit-spirv-directly -capability spvAtomicFloat16VectorNV -DTEST_UNSUPPORTED_VECTOR_WIDTH

// Pointer-form fp16 vector checks. These paths are emitted as vector atomics, so
// they must diagnose missing spvAtomicFloat16VectorNV when only scalar fp16 add is enabled.
// Runtime coverage for these pointer-form helpers would require SPIR-V variable pointers,
// which existing tests keep disabled on current GCP runners.
//DIAGNOSTIC_TEST:SIMPLE(diag=POINTER_EMULATED_NO_VECTOR,non-exhaustive): -target spirv -entry computeMain -stage compute -emit-spirv-directly -restrictive-capability-check -capability spvAtomicFloat16AddEXT -DTEST_POINTER_EMULATED
//DIAGNOSTIC_TEST:SIMPLE(diag=POINTER_F16X2_NO_VECTOR,non-exhaustive): -target spirv -entry computeMain -stage compute -emit-spirv-directly -restrictive-capability-check -capability spvAtomicFloat16AddEXT -DTEST_POINTER_F16X2

// Positive codegen checks for ignored capabilities, scalar/vector SPIR-V, and CUDA.
//TEST:SIMPLE(filecheck=IGNORE_CAPS): -target spirv-asm -entry computeMain -stage compute -emit-spirv-directly -restrictive-capability-check -ignore-capabilities -capability spirv_1_5
//TEST:SIMPLE(filecheck=BOTH): -target spirv -entry computeMain -stage compute -emit-spirv-directly -capability spvAtomicFloat16AddEXT -capability spvAtomicFloat16VectorNV
//TEST:SIMPLE(filecheck=VECTOR): -target spirv -entry computeMain -stage compute -emit-spirv-directly -capability spvAtomicFloat16VectorNV -DTEST_DIRECT_VECTOR_ATOMIC
//TEST:SIMPLE(filecheck=VECTOR4): -target spirv -entry computeMain -stage compute -emit-spirv-directly -capability spvAtomicFloat16VectorNV -DTEST_DIRECT_VECTOR4_ATOMIC
//TEST:SIMPLE(filecheck=VECTOR_MIN_MAX): -target spirv -entry computeMain -stage compute -emit-spirv-directly -capability spvAtomicFloat16VectorNV -DTEST_DIRECT_VECTOR_MIN_MAX
//TEST:SIMPLE(filecheck=VECTOR_EXCHANGE): -target spirv -entry computeMain -stage compute -emit-spirv-directly -capability spvAtomicFloat16VectorNV -DTEST_DIRECT_VECTOR_EXCHANGE
//TEST:SIMPLE(filecheck=SCALAR_EXCHANGE): -target spirv -entry computeMain -stage compute -emit-spirv-directly -DTEST_DIRECT_SCALAR_EXCHANGE
//TEST:SIMPLE(filecheck=SCALAR_HALF_EXCHANGE): -target spirv -entry computeMain -stage compute -emit-spirv-directly -DTEST_DIRECT_SCALAR_HALF_EXCHANGE
//TEST:SIMPLE(filecheck=EMULATED): -target spirv -entry computeMain -stage compute -emit-spirv-directly -DTEST_EMULATED
//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute -capability cuda_sm_7_0

//TEST_INPUT:ubuffer(stride=4, count=8):name=tmpBuffer
RWByteAddressBuffer tmpBuffer;
//TEST_INPUT:ubuffer(stride=4, count=32):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;
//TEST_INPUT:ubuffer(stride=2, count=4):name=halfBuffer
RWStructuredBuffer<half> halfBuffer;
//TEST_INPUT:ubuffer(stride=4, count=4):name=vectorBuffer
RWStructuredBuffer<half2> vectorBuffer;
//TEST_INPUT:ubuffer(stride=8, count=4):name=vector4Buffer
RWStructuredBuffer<half4> vector4Buffer;
#ifdef TEST_UNSUPPORTED_VECTOR_WIDTH
RWStructuredBuffer<half3> unsupportedVectorBuffer;
#endif
//TEST_INPUT:ubuffer(stride=4, count=1):name=uintBuffer
RWStructuredBuffer<uint> uintBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
#ifdef TEST_RUNTIME_SCALAR
    half originalLow;
    half originalHigh;
    tmpBuffer.Store<half>(0, 2.0h);
    tmpBuffer.Store<half>(2, 4.0h);
    tmpBuffer.InterlockedAddF16(0, 1.0h, originalLow);
    tmpBuffer.InterlockedAddF16(2, 3.0h, originalHigh);

    outputBuffer[0] = float(originalLow);
    outputBuffer[1] = float(originalHigh);
    outputBuffer[2] = float(tmpBuffer.Load<half>(0));
    outputBuffer[3] = float(tmpBuffer.Load<half>(2));
    // RUNTIME_SCALAR: type: float
    // RUNTIME_SCALAR-NEXT: 2.000000
    // RUNTIME_SCALAR-NEXT: 4.000000
    // RUNTIME_SCALAR-NEXT: 3.000000
    // RUNTIME_SCALAR-NEXT: 7.000000
#elif defined(TEST_RUNTIME_DIRECT)
    vectorBuffer[0] = half2(2.0h, 4.0h);
    vectorBuffer[1] = half2(5.0h, 1.0h);
    vectorBuffer[2] = half2(5.0h, 1.0h);
    vectorBuffer[3] = half2(8.0h, 9.0h);

    half2 originalAdd = __atomic_add(vectorBuffer[0], half2(1.0h, 3.0h));
    half2 originalMin = __atomic_min(vectorBuffer[1], half2(3.0h, 2.0h));
    half2 originalMax = __atomic_max(vectorBuffer[2], half2(3.0h, 2.0h));
    half2 originalExchange = __atomic_exchange(vectorBuffer[3], half2(6.0h, 7.0h));

    outputBuffer[0] = float(originalAdd.x);
    outputBuffer[1] = float(originalAdd.y);
    outputBuffer[2] = float(vectorBuffer[0].x);
    outputBuffer[3] = float(vectorBuffer[0].y);
    outputBuffer[4] = float(originalMin.x);
    outputBuffer[5] = float(originalMin.y);
    outputBuffer[6] = float(vectorBuffer[1].x);
    outputBuffer[7] = float(vectorBuffer[1].y);
    outputBuffer[8] = float(originalMax.x);
    outputBuffer[9] = float(originalMax.y);
    outputBuffer[10] = float(vectorBuffer[2].x);
    outputBuffer[11] = float(vectorBuffer[2].y);
    outputBuffer[12] = float(originalExchange.x);
    outputBuffer[13] = float(originalExchange.y);
    outputBuffer[14] = float(vectorBuffer[3].x);
    outputBuffer[15] = float(vectorBuffer[3].y);
    // RUNTIME_DIRECT: type: float
    // RUNTIME_DIRECT-NEXT: 2.000000
    // RUNTIME_DIRECT-NEXT: 4.000000
    // RUNTIME_DIRECT-NEXT: 3.000000
    // RUNTIME_DIRECT-NEXT: 7.000000
    // RUNTIME_DIRECT-NEXT: 5.000000
    // RUNTIME_DIRECT-NEXT: 1.000000
    // RUNTIME_DIRECT-NEXT: 3.000000
    // RUNTIME_DIRECT-NEXT: 1.000000
    // RUNTIME_DIRECT-NEXT: 5.000000
    // RUNTIME_DIRECT-NEXT: 1.000000
    // RUNTIME_DIRECT-NEXT: 5.000000
    // RUNTIME_DIRECT-NEXT: 2.000000
    // RUNTIME_DIRECT-NEXT: 8.000000
    // RUNTIME_DIRECT-NEXT: 9.000000
    // RUNTIME_DIRECT-NEXT: 6.000000
    // RUNTIME_DIRECT-NEXT: 7.000000
#elif defined(TEST_DIRECT_VECTOR_ATOMIC)
    __atomic_add(vectorBuffer[0], half2(1.0h, 2.0h));
// DIRECT_NO_VECTOR: entry point uses capabilities not in specified profile
// DIRECT_NO_VECTOR: Missing capabilities are: 'spvAtomicFloat16VectorNV'
#elif defined(TEST_DIRECT_VECTOR4_ATOMIC)
    __atomic_add(vector4Buffer[0], half4(1.0h, 2.0h, 3.0h, 4.0h));
// DIRECT_VECTOR4_NO_VECTOR: entry point uses capabilities not in specified profile
// DIRECT_VECTOR4_NO_VECTOR: Missing capabilities are: 'spvAtomicFloat16VectorNV'
#elif defined(TEST_DIRECT_VECTOR_MIN_MAX)
    __atomic_min(vectorBuffer[0], half2(1.0h, 2.0h));
// DIRECT_MIN_MAX_NO_VECTOR: entry point uses capabilities not in specified profile
// DIRECT_MIN_MAX_NO_VECTOR: Missing capabilities are: 'spvAtomicFloat16VectorNV'
    __atomic_max(vectorBuffer[1], half2(3.0h, 4.0h));
#elif defined(TEST_DIRECT_VECTOR_EXCHANGE)
    half2 originalValue = __atomic_exchange(vectorBuffer[0], half2(1.0h, 2.0h));
// DIRECT_EXCHANGE_NO_VECTOR: entry point uses capabilities not in specified profile
// DIRECT_EXCHANGE_NO_VECTOR: Missing capabilities are: 'spvAtomicFloat16VectorNV'
    outputBuffer[0] = float(originalValue.x + originalValue.y);
#elif defined(TEST_DIRECT_SCALAR_EXCHANGE)
    uint originalValue = __atomic_exchange(uintBuffer[0], 1u);
    outputBuffer[0] = float(originalValue);
#elif defined(TEST_DIRECT_SCALAR_HALF_EXCHANGE)
    half originalValue = __atomic_exchange(halfBuffer[0], 1.0h);
    outputBuffer[0] = float(originalValue);
#elif defined(TEST_UNSUPPORTED_VECTOR_WIDTH)
    __atomic_add(unsupportedVectorBuffer[0], half3(1.0h, 2.0h, 3.0h));
// UNSUPPORTED_VECTOR_WIDTH: error[E50013]: invalid SPIR-V fp16 vector atomic width
// UNSUPPORTED_VECTOR_WIDTH: SPIR-V fp16 vector atomics only support half2 and half4.
#elif defined(TEST_EMULATED)
    half originalValue;
    tmpBuffer.InterlockedAddF16Emulated(0, 1.0h, originalValue);
    outputBuffer[0] = float(originalValue);
    tmpBuffer.InterlockedAddF16Emulated(2, 1.0h, originalValue);
    outputBuffer[1] = float(originalValue);
#elif defined(TEST_POINTER_EMULATED)
    half originalValue;
    InterlockedAddF16Emulated(&halfBuffer[0], 1.0h, originalValue);
// POINTER_EMULATED_NO_VECTOR: entry point uses capabilities not in specified profile
// POINTER_EMULATED_NO_VECTOR: Missing capabilities are: 'spvAtomicFloat16VectorNV'
    outputBuffer[0] = float(originalValue);
#elif defined(TEST_POINTER_F16X2)
    half2 originalValue;
    InterlockedAddF16x2(&vectorBuffer[0], half2(1.0h, 2.0h), originalValue);
// POINTER_F16X2_NO_VECTOR: entry point uses capabilities not in specified profile
// POINTER_F16X2_NO_VECTOR: Missing capabilities are: 'spvAtomicFloat16VectorNV'
    outputBuffer[0] = float(originalValue.x + originalValue.y);
#else
    half originalValue;
    tmpBuffer.InterlockedAddF16(0, 1.0h, originalValue);
// NO_FP16_ATOMIC: entry point uses capabilities not in specified profile
// NO_FP16_ATOMIC: Missing capabilities are: 'spvAtomicFloat16AddEXT'
// VECTOR_ONLY: entry point uses capabilities not in specified profile
// VECTOR_ONLY: Missing capabilities are: 'spvAtomicFloat16AddEXT'
// IGNORE_CAPS-NOT: entry point uses capabilities not in specified profile
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
// VECTOR: OpAtomicFAddEXT %v2half

// VECTOR4-NOT: OpCapability AtomicFloat16AddEXT
// VECTOR4-NOT: OpExtension "SPV_EXT_shader_atomic_float16_add"
// VECTOR4: OpCapability AtomicFloat16VectorNV
// VECTOR4: OpExtension "SPV_NV_shader_atomic_fp16_vector"
// VECTOR4: OpAtomicFAddEXT %v4half
// VECTOR4-NOT: OpCapability AtomicFloat16AddEXT
// VECTOR4-NOT: OpExtension "SPV_EXT_shader_atomic_float16_add"

// VECTOR_MIN_MAX-NOT: OpCapability AtomicFloat16AddEXT
// VECTOR_MIN_MAX-NOT: OpExtension "SPV_EXT_shader_atomic_float16_add"
// VECTOR_MIN_MAX: OpCapability AtomicFloat16VectorNV
// VECTOR_MIN_MAX: OpExtension "SPV_NV_shader_atomic_fp16_vector"
// VECTOR_MIN_MAX: OpAtomicFMinEXT %v2half
// VECTOR_MIN_MAX: OpAtomicFMaxEXT %v2half
// VECTOR_MIN_MAX-NOT: OpCapability AtomicFloat16AddEXT
// VECTOR_MIN_MAX-NOT: OpExtension "SPV_EXT_shader_atomic_float16_add"

// VECTOR_EXCHANGE-NOT: OpCapability AtomicFloat16AddEXT
// VECTOR_EXCHANGE-NOT: OpExtension "SPV_EXT_shader_atomic_float16_add"
// VECTOR_EXCHANGE: OpCapability AtomicFloat16VectorNV
// VECTOR_EXCHANGE: OpExtension "SPV_NV_shader_atomic_fp16_vector"
// VECTOR_EXCHANGE: OpAtomicExchange %v2half
// VECTOR_EXCHANGE-NOT: OpCapability AtomicFloat16AddEXT
// VECTOR_EXCHANGE-NOT: OpExtension "SPV_EXT_shader_atomic_float16_add"

// SCALAR_EXCHANGE-NOT: OpCapability AtomicFloat16VectorNV
// SCALAR_EXCHANGE-NOT: OpExtension "SPV_NV_shader_atomic_fp16_vector"
// SCALAR_EXCHANGE: OpAtomicExchange %uint
// SCALAR_EXCHANGE-NOT: OpCapability AtomicFloat16VectorNV
// SCALAR_EXCHANGE-NOT: OpExtension "SPV_NV_shader_atomic_fp16_vector"

// SCALAR_HALF_EXCHANGE-NOT: OpCapability AtomicFloat16VectorNV
// SCALAR_HALF_EXCHANGE-NOT: OpExtension "SPV_NV_shader_atomic_fp16_vector"
// SCALAR_HALF_EXCHANGE: OpAtomicExchange %half
// SCALAR_HALF_EXCHANGE-NOT: OpCapability AtomicFloat16VectorNV
// SCALAR_HALF_EXCHANGE-NOT: OpExtension "SPV_NV_shader_atomic_fp16_vector"

// EMULATED-NOT: OpCapability AtomicFloat16AddEXT
// EMULATED-NOT: OpExtension "SPV_EXT_shader_atomic_float16_add"
// EMULATED: OpCapability AtomicFloat16VectorNV
// EMULATED: OpExtension "SPV_NV_shader_atomic_fp16_vector"
// EMULATED-DAG: [[LOW:%[0-9]+]] = OpConstantComposite %v2half %half_0x1p_0 %half_0x0p_0
// EMULATED-DAG: [[HIGH:%[0-9]+]] = OpConstantComposite %v2half %half_0x0p_0 %half_0x1p_0
// EMULATED-NOT: OpAtomicFAddEXT
// EMULATED: [[LOW_ATOMIC:%[0-9]+]] = OpAtomicFAddEXT %v2half {{%[0-9]+}} %uint_1 %uint_0 [[LOW]]
// EMULATED: OpCompositeExtract %half [[LOW_ATOMIC]] 0
// EMULATED: [[HIGH_ATOMIC:%[0-9]+]] = OpAtomicFAddEXT %v2half {{%[0-9]+}} %uint_1 %uint_0 [[HIGH]]
// EMULATED: OpCompositeExtract %half [[HIGH_ATOMIC]] 1
// EMULATED-NOT: OpAtomicFAddEXT

// CUDA-COUNT-5: atomicAdd(
