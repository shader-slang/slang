//TEST:SIMPLE(filecheck=NEGATIVE): -target spirv -entry computeMain -stage compute -emit-spirv-directly
//TEST:SIMPLE(filecheck=POSITIVE): -target spirv -entry computeMain -stage compute -emit-spirv-directly -DPOSITIVE_VECTOR_ATOMIC
//TEST:SIMPLE(filecheck=POSITIVE_MIN_MAX): -target spirv -entry computeMain -stage compute -emit-spirv-directly -DPOSITIVE_VECTOR_MIN_MAX
//DIAGNOSTIC_TEST:SIMPLE(diag=POSITIVE_WARN,non-exhaustive): -target spirv -profile spirv_1_5 -entry computeMain -stage compute -emit-spirv-directly -DPOSITIVE_VECTOR_ATOMIC
//TEST:SIMPLE(filecheck=POSITIVE_IGNORE_CAPS): -target spirv -profile spirv_1_5 -entry computeMain -stage compute -emit-spirv-directly -ignore-capabilities -DPOSITIVE_VECTOR_ATOMIC

// Requiring the GLSL extension should not declare the NV vector capability until a vector
// atomic operation needs it.
RWStructuredBuffer<half2> vectorBuffer;

[require(GL_NV_shader_atomic_fp16_vector)]
[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
#ifdef POSITIVE_VECTOR_ATOMIC
    __atomic_add(vectorBuffer[0], half2(1.0h, 2.0h));
// POSITIVE_WARN: profile implicitly upgraded
// POSITIVE_WARN: spvAtomicFloat16VectorNV
#elif defined(POSITIVE_VECTOR_MIN_MAX)
    __atomic_min(vectorBuffer[0], half2(1.0h, 2.0h));
    __atomic_max(vectorBuffer[1], half2(3.0h, 4.0h));
#else
    vectorBuffer[0] = half2(1.0h, 2.0h);
#endif
}

// NEGATIVE-NOT: OpCapability AtomicFloat16VectorNV
// NEGATIVE-NOT: OpExtension "SPV_NV_shader_atomic_fp16_vector"
// NEGATIVE: OpMemoryModel

// POSITIVE: OpCapability AtomicFloat16VectorNV
// POSITIVE: OpExtension "SPV_NV_shader_atomic_fp16_vector"
// POSITIVE: OpAtomicFAddEXT %v2half

// POSITIVE_IGNORE_CAPS-NOT: warning[E41012]
// POSITIVE_IGNORE_CAPS: OpCapability AtomicFloat16VectorNV
// POSITIVE_IGNORE_CAPS: OpExtension "SPV_NV_shader_atomic_fp16_vector"
// POSITIVE_IGNORE_CAPS: OpAtomicFAddEXT %v2half

// POSITIVE_MIN_MAX: OpCapability AtomicFloat16VectorNV
// POSITIVE_MIN_MAX: OpExtension "SPV_NV_shader_atomic_fp16_vector"
// POSITIVE_MIN_MAX: OpAtomicFMinEXT %v2half
// POSITIVE_MIN_MAX: OpAtomicFMaxEXT %v2half
