// See shader-slang/slang#9910. A storage-buffer 8/16-bit access should require the narrow
// StorageBuffer{8,16}BitAccess capability, not the broad UniformAndStorageBuffer variant, which a
// storage-only shader does not need; a uniform (constant-buffer) access does need the broad one,
// and when one type is used both ways the broad capability subsumes the narrow one. The cases below
// cover storage-only (OUT_*), uniform-only (IN_*), independent-size mixed (MIX), and shared-struct
// (SHARED) usage. The storage/mixed/shared cases use the default profile so a RWStructuredBuffer is
// emitted with the modern StorageBuffer storage class that this fix concerns; the uniform-only IN_*
// cases pin -profile spirv_1_3, but a constant buffer is Uniform and requires the broad capability
// under either profile, so their profile is not load-bearing (MIX shows a default-profile uniform
// access still requiring broad). Each case's expected capability is stated with its own directives.
//
// Each configuration with both presence and absence expectations is checked by two independent
// FileCheck invocations: a "present" prefix asserting the required capability is emitted, and an
// absence-only "-NOT" prefix (no positive directive) that scans the ENTIRE module for the forbidden
// capability. Keeping the forbidden checks in their own invocation avoids the interval blind spots
// that arise when -NOT directives are interleaved with positive matches, so the result is
// independent of the order in which the emitter adds capabilities.

//TEST:SIMPLE(filecheck=UNI8): -target spirv -profile spirv_1_3 -DIN_UINT8
//TEST:SIMPLE(filecheck=UNI8_NOT): -target spirv -profile spirv_1_3 -DIN_UINT8
//TEST:SIMPLE(filecheck=UNI16): -target spirv -profile spirv_1_3 -DIN_UINT16
//TEST:SIMPLE(filecheck=UNI16_NOT): -target spirv -profile spirv_1_3 -DIN_UINT16
//TEST:SIMPLE(filecheck=UNI16): -target spirv -profile spirv_1_3 -DIN_HALF
//TEST:SIMPLE(filecheck=UNI16_NOT): -target spirv -profile spirv_1_3 -DIN_HALF
//TEST:SIMPLE(filecheck=UNIBOTH): -target spirv -profile spirv_1_3 -DIN_UINT8 -DIN_HALF
//TEST:SIMPLE(filecheck=UNIBOTH_NOT): -target spirv -profile spirv_1_3 -DIN_UINT8 -DIN_HALF

//TEST:SIMPLE(filecheck=SSBO8): -target spirv -DOUT_UINT8
//TEST:SIMPLE(filecheck=SSBO8_NOT): -target spirv -DOUT_UINT8
//TEST:SIMPLE(filecheck=SSBO16): -target spirv -DOUT_UINT16
//TEST:SIMPLE(filecheck=SSBO16_NOT): -target spirv -DOUT_UINT16
//TEST:SIMPLE(filecheck=SSBO16): -target spirv -DOUT_HALF
//TEST:SIMPLE(filecheck=SSBO16_NOT): -target spirv -DOUT_HALF
//TEST:SIMPLE(filecheck=SSBO16): -target spirv -DOUT_HALF -DATOMIC
//TEST:SIMPLE(filecheck=SSBO16_NOT): -target spirv -DOUT_HALF -DATOMIC

// Uniform (16-bit) and storage (8-bit) of independent sizes -> the two capabilities coexist.
//TEST:SIMPLE(filecheck=MIX): -target spirv -DIN_HALF -DOUT_UINT8
//TEST:SIMPLE(filecheck=MIX_NOT): -target spirv -DIN_HALF -DOUT_UINT8

// The SAME struct type bound through both a uniform and a storage buffer -> the broad capability
// subsumes the narrow one, so only the broad capability is emitted (never both, and never only the
// narrow one -- under-requiring would be invalid SPIR-V). Both bodies below assert broad-only; each
// is intended to exercise a different internal path, but the checks cannot tell the paths apart
// (the minimal capability set is broad-only either way), so the coverage is best-effort:
//   * default body: the leading `sharedOut[0].data = sharedIn.data;` touches both buffers, so which
//     pointer type is emitted first is not forced here. In practice the storage access is emitted
//     first, so the narrow capability is added and then unlinked when the uniform access requires
//     the broad one (the removal path); if that order did not hold, this case would still pass.
//   * SHARED_UNIFORM_FIRST body: reads `sharedIn.index` first, so the uniform access (and the broad
//     capability) is emitted before the storage access and the narrow capability is never added
//     (the skip path).
// Either way a broken subsumption would leave the narrow capability present and fail the
// SHARED*_NOT checks.
//TEST:SIMPLE(filecheck=SHARED8): -target spirv -DSHARED -DSHARED_UINT8
//TEST:SIMPLE(filecheck=SHARED8_NOT): -target spirv -DSHARED -DSHARED_UINT8
//TEST:SIMPLE(filecheck=SHARED16): -target spirv -DSHARED -DSHARED_UINT16
//TEST:SIMPLE(filecheck=SHARED16_NOT): -target spirv -DSHARED -DSHARED_UINT16
//TEST:SIMPLE(filecheck=SHARED8): -target spirv -DSHARED -DSHARED_UINT8 -DSHARED_UNIFORM_FIRST
//TEST:SIMPLE(filecheck=SHARED8_NOT): -target spirv -DSHARED -DSHARED_UINT8 -DSHARED_UNIFORM_FIRST
//TEST:SIMPLE(filecheck=SHARED16): -target spirv -DSHARED -DSHARED_UINT16 -DSHARED_UNIFORM_FIRST
//TEST:SIMPLE(filecheck=SHARED16_NOT): -target spirv -DSHARED -DSHARED_UINT16 -DSHARED_UNIFORM_FIRST

//TEST:SIMPLE(filecheck=NONE): -target spirv

//UNI8: OpCapability UniformAndStorageBuffer8BitAccess
//UNI8_NOT-NOT: OpCapability StorageBuffer8BitAccess
//UNI16: OpCapability UniformAndStorageBuffer16BitAccess
//UNI16_NOT-NOT: OpCapability StorageBuffer16BitAccess
//UNIBOTH-DAG: OpCapability UniformAndStorageBuffer8BitAccess
//UNIBOTH-DAG: OpCapability UniformAndStorageBuffer16BitAccess
//UNIBOTH_NOT-NOT: OpCapability StorageBuffer8BitAccess
//UNIBOTH_NOT-NOT: OpCapability StorageBuffer16BitAccess

//SSBO8: OpCapability StorageBuffer8BitAccess
//SSBO8_NOT-NOT: OpCapability UniformAndStorageBuffer8BitAccess
//SSBO16: OpCapability StorageBuffer16BitAccess
//SSBO16_NOT-NOT: OpCapability UniformAndStorageBuffer16BitAccess

//MIX-DAG: OpCapability UniformAndStorageBuffer16BitAccess
//MIX-DAG: OpCapability StorageBuffer8BitAccess
//MIX_NOT-NOT: OpCapability StorageBuffer16BitAccess
//MIX_NOT-NOT: OpCapability UniformAndStorageBuffer8BitAccess

//SHARED8: OpCapability UniformAndStorageBuffer8BitAccess
//SHARED8_NOT-NOT: OpCapability StorageBuffer8BitAccess
//SHARED16: OpCapability UniformAndStorageBuffer16BitAccess
//SHARED16_NOT-NOT: OpCapability StorageBuffer16BitAccess

//NONE-NOT: OpCapability UniformAndStorageBuffer8BitAccess
//NONE-NOT: OpCapability UniformAndStorageBuffer16BitAccess
//NONE-NOT: OpCapability StorageBuffer8BitAccess
//NONE-NOT: OpCapability StorageBuffer16BitAccess


#if defined(SHARED)

struct Shared {
    float4 data;
#if defined(SHARED_UINT8)
    uint8_t index;
#else
    uint16_t index;
#endif
};
ConstantBuffer<Shared> sharedIn;
RWStructuredBuffer<Shared> sharedOut;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
#if defined(SHARED_UNIFORM_FIRST)
    // Read the uniform 8/16-bit member first so the uniform pointer type (and hence the broad
    // capability) is emitted before the storage-buffer access, exercising the broad-first ordering.
    let index = sharedIn.index;
    sharedOut[0].data = sharedIn.data;
    sharedOut[1].index = index;
#else
    // No forced ordering: this leading store touches both buffers, so removal-vs-skip depends on
    // intra-statement emission order (best-effort removal-path coverage -- see the SHARED header).
    sharedOut[0].data = sharedIn.data;
    sharedOut[1].index = sharedIn.index;
#endif
}

#else

uniform struct {
#if defined(IN_HALF)
    half4 data;
#else
    float4 data;
#endif
#if defined(IN_UINT8)
    uint8_t index;
#elif defined(IN_UINT16)
    uint16_t index;
#else
    uint32_t index;
#endif
} inputBuffer;

#if defined(OUT_HALF)
#define OUT_FLOAT_TYPE half
#else
#define OUT_FLOAT_TYPE float
#endif
#if defined(OUT_UINT8)
#define OUT_UINT_TYPE uint8_t
#elif defined(OUT_UINT16)
#define OUT_UINT_TYPE uint16_t
#else
#define OUT_UINT_TYPE uint32_t
#endif

struct st {
#if defined(ATOMIC)
    Atomic<OUT_FLOAT_TYPE> data;
#else
    OUT_FLOAT_TYPE data;
#endif
    OUT_UINT_TYPE index;
};
RWStructuredBuffer<st> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0].data = OUT_FLOAT_TYPE(inputBuffer.data.x);
    outputBuffer[1].index = OUT_UINT_TYPE(inputBuffer.index);
}

#endif
