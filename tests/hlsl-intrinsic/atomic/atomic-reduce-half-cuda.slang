// atomic-reduce-half-cuda.slang
// Tests for atomic reduction intrinsics with half and half2 types (CUDA only)
// These intrinsics perform atomic operations without returning the old value.
// On CUDA, they use the PTX `red` instruction with f16/f16x2 types.
//
// This test is CUDA-only because Vulkan requires SPV_EXT_shader_atomic_float16_add
// which is not widely supported by drivers.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-cuda -compute -shaderobj -output-using-type

// half add reduction target
//TEST_INPUT:ubuffer(data=[0], stride=4):name=halfAddTarget
RWStructuredBuffer<half> halfAddTarget;

// half2 (vector<half, 2>) add reduction target
//TEST_INPUT:ubuffer(data=[0 0], stride=4):name=half2AddTarget
RWStructuredBuffer<half2> half2AddTarget;

//TEST_INPUT: ubuffer(data=[0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

static const int NUM_THREADS = 64;

[numthreads(NUM_THREADS, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;

    // === Test __atomic_reduce_add with half ===
    // Each thread adds 0.125h, result should be 8.0h (64 * 0.125 = 8.0)
    __atomic_reduce_add(halfAddTarget[0], half(0.125f));

    // === Test __atomic_reduce_add with half2 ===
    // Each thread adds (0.125h, 0.25h), results should be (8.0h, 16.0h)
    vector<half, 2> half2Val = vector<half, 2>(half(0.125f), half(0.25f));
    __atomic_reduce_add(half2AddTarget[0], half2Val);

    AllMemoryBarrierWithGroupSync();

    // Only thread 0 writes results
    if (tid == 0)
    {
        outputBuffer[0] = int(halfAddTarget[0]);       // Expected: 8 (from 8.0h)
        outputBuffer[1] = int(half2AddTarget[0].x);    // Expected: 8 (from 8.0h)
        outputBuffer[2] = int(half2AddTarget[0].y);    // Expected: 16 (from 16.0h)
    }
}

// BUF: 8
// BUF-NEXT: 8
// BUF-NEXT: 16
