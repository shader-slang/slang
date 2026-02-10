// atomic-reduce-intrinsics.slang
// Tests for atomic reduction intrinsics (__atomic_reduce_xxx)
// These intrinsics perform atomic operations without returning the old value.
// On CUDA, they use the PTX `red` instruction which is faster than `atom`.
// On other targets, they fall back to regular atomic operations.
//
// This test launches 64 threads that all atomically reduce to single memory locations,
// testing true concurrent atomic behavior.
//
// NOTE: PTX `red` instruction only supports min/max for integer types.
// Floating-point min/max are NOT supported by the `red` instruction.
// For floating-point min/max atomics, use __atomic_min/__atomic_max instead.
//
// NOTE: half/bfloat16 atomic operations are only tested in CUDA-specific tests
// because Vulkan requires SPV_EXT_shader_atomic_float16_add which is not widely supported.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-cuda -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-slang -compute -dx12 -profile cs_6_0 -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -emit-spirv-directly -compute -shaderobj -output-using-type -render-feature hardware-device

// Integer reduction targets - all threads reduce to index 0
//TEST_INPUT:ubuffer(data=[0], stride=4):name=uintAddTarget
RWStructuredBuffer<uint> uintAddTarget;
//TEST_INPUT:ubuffer(data=[0], stride=4):name=intAddTarget
RWStructuredBuffer<int> intAddTarget;

// Floating-point add reduction targets (PTX `red` supports add for floats)
//TEST_INPUT:ubuffer(data=[0.0], stride=4):name=floatAddTarget
RWStructuredBuffer<float> floatAddTarget;
//TEST_INPUT:ubuffer(data=[0.0 0.0], stride=8):name=doubleAddTarget
RWStructuredBuffer<double> doubleAddTarget;

// Integer Min/Max targets - initialized to values that will be reduced
// NOTE: PTX `red` only supports min/max for integer types (.u32, .u64, .s32, .s64)
//TEST_INPUT:ubuffer(data=[1000], stride=4):name=uintMinTarget
RWStructuredBuffer<uint> uintMinTarget;
//TEST_INPUT:ubuffer(data=[0], stride=4):name=uintMaxTarget
RWStructuredBuffer<uint> uintMaxTarget;
//TEST_INPUT:ubuffer(data=[1000], stride=4):name=intMinTarget
RWStructuredBuffer<int> intMinTarget;
//TEST_INPUT:ubuffer(data=[-1000], stride=4):name=intMaxTarget
RWStructuredBuffer<int> intMaxTarget;

// Bitwise targets
//TEST_INPUT:ubuffer(data=[0xFFFFFFFF], stride=4):name=uintAndTarget
RWStructuredBuffer<uint> uintAndTarget;
//TEST_INPUT:ubuffer(data=[0], stride=4):name=uintOrTarget
RWStructuredBuffer<uint> uintOrTarget;
//TEST_INPUT:ubuffer(data=[0], stride=4):name=uintXorTarget
RWStructuredBuffer<uint> uintXorTarget;

// Inc/Dec targets
//TEST_INPUT:ubuffer(data=[0], stride=4):name=uintIncTarget
RWStructuredBuffer<uint> uintIncTarget;
//TEST_INPUT:ubuffer(data=[100], stride=4):name=intDecTarget
RWStructuredBuffer<int> intDecTarget;

//TEST_INPUT: ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

static const int NUM_THREADS = 64;

[numthreads(NUM_THREADS, 1, 1)]
[shader("compute")]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;

    // === Test __atomic_reduce_add ===
    // Each of 64 threads adds 1, result should be 64
    __atomic_reduce_add(uintAddTarget[0], uint(1));
    __atomic_reduce_add(intAddTarget[0], 1);

    // Floating-point atomic add (PTX red.add supports f32, f64)
    // Each thread adds 0.5f, result should be 32.0f
    __atomic_reduce_add(floatAddTarget[0], 0.5f);
    // Each thread adds 0.25, result should be 16.0
    __atomic_reduce_add(doubleAddTarget[0], 0.25);

    // === Test __atomic_reduce_min (integers only) ===
    // Each thread tries to set min to its thread id
    // Result should be 0 (thread 0's value)
    __atomic_reduce_min(uintMinTarget[0], uint(tid));
    __atomic_reduce_min(intMinTarget[0], int(tid));

    // === Test __atomic_reduce_max (integers only) ===
    // Each thread tries to set max to its thread id
    // Result should be 63 (thread 63's value)
    __atomic_reduce_max(uintMaxTarget[0], uint(tid));
    __atomic_reduce_max(intMaxTarget[0], int(tid));

    // === Test __atomic_reduce_and ===
    // Each thread ANDs with (0xFFFFFFFF ^ (1 << (tid % 32)))
    // After all threads, bits 0-31 should be cleared for threads 0-31,
    // and threads 32-63 will clear the same bits again (no effect)
    // Result: 0xFFFFFFFF with bits 0-31 cleared = 0x00000000
    __atomic_reduce_and(uintAndTarget[0], ~(1u << (tid % 32)));

    // === Test __atomic_reduce_or ===
    // Each thread ORs with (1 << (tid % 32))
    // Result should have bits 0-31 set = 0xFFFFFFFF
    __atomic_reduce_or(uintOrTarget[0], 1u << (tid % 32));

    // === Test __atomic_reduce_xor ===
    // Each thread XORs with (1 << (tid % 32))
    // Threads 0-31 set bits, threads 32-63 clear them
    // Result should be 0
    __atomic_reduce_xor(uintXorTarget[0], 1u << (tid % 32));

    // === Test __atomic_reduce_inc ===
    // 64 threads each increment by 1, result should be 64
    __atomic_reduce_inc(uintIncTarget[0]);

    // === Test __atomic_reduce_dec ===
    // 64 threads each decrement by 1, starting from 100, result should be 36
    __atomic_reduce_dec(intDecTarget[0]);

    AllMemoryBarrierWithGroupSync();

    // Only thread 0 writes results
    if (tid == 0)
    {
        int idx = 0;
        outputBuffer[idx++] = int(uintAddTarget[0]);      // Expected: 64
        outputBuffer[idx++] = intAddTarget[0];            // Expected: 64
        outputBuffer[idx++] = int(floatAddTarget[0]);     // Expected: 32 (from 32.0f)
        outputBuffer[idx++] = int(doubleAddTarget[0]);    // Expected: 16 (from 16.0)
        outputBuffer[idx++] = int(uintMinTarget[0]);      // Expected: 0
        outputBuffer[idx++] = int(uintMaxTarget[0]);      // Expected: 63
        outputBuffer[idx++] = intMinTarget[0];            // Expected: 0
        outputBuffer[idx++] = intMaxTarget[0];            // Expected: 63
        outputBuffer[idx++] = int(uintAndTarget[0]);      // Expected: 0
        outputBuffer[idx++] = int(uintOrTarget[0]);       // Expected: -1 (0xFFFFFFFF as signed)
        outputBuffer[idx++] = int(uintXorTarget[0]);      // Expected: 0
        outputBuffer[idx++] = int(uintIncTarget[0]);      // Expected: 64
        outputBuffer[idx++] = intDecTarget[0];            // Expected: 36
    }
}

// BUF:      64
// BUF-NEXT: 64
// BUF-NEXT: 32
// BUF-NEXT: 16
// BUF-NEXT: 0
// BUF-NEXT: 63
// BUF-NEXT: 0
// BUF-NEXT: 63
// BUF-NEXT: 0
// BUF-NEXT: -1
// BUF-NEXT: 0
// BUF-NEXT: 64
// BUF-NEXT: 36
