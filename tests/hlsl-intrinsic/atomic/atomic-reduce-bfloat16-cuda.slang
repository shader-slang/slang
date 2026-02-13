// atomic-reduce-bfloat16-cuda.slang
// Tests for atomic reduction intrinsics with BFloat16 types (CUDA only)
// These intrinsics perform atomic operations without returning the old value.
// On CUDA, they use the PTX `red` instruction with bf16/bf16x2 types.
//
// This test launches 64 threads that all atomically reduce to single memory locations,
// testing true concurrent atomic behavior.
//
// NOTE: PTX `red` instruction only supports `add` operation for bf16/bf16x2.
// Min/max operations are NOT supported by the `red` instruction for floating-point types.
//
// Requires SM 9.0 (Hopper) or newer GPU architecture.

// TODO: this will need PR for slang-rhi
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-cuda -compute -shaderobj -output-using-type -render-feature atomic-bfloat16

// BFloat16 add reduction target
//TEST_INPUT:ubuffer(data=[0], stride=4):name=bf16AddTarget
RWStructuredBuffer<BFloat16> bf16AddTarget;

// vector<BFloat16, 2> add reduction target (packed as 32-bit)
//TEST_INPUT:ubuffer(data=[0], stride=4):name=bf16x2AddTarget
RWStructuredBuffer<vector<BFloat16, 2>> bf16x2AddTarget;

//TEST_INPUT: ubuffer(data=[0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

static const int NUM_THREADS = 64;

[numthreads(NUM_THREADS, 1, 1)]
[shader("compute")]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;

    // === Test __atomic_reduce_add with BFloat16 ===
    // Each thread adds 0.0625bf, result should be 4.0bf (64 * 0.0625 = 4.0)
    __atomic_reduce_add(bf16AddTarget[0], BFloat16(0.0625f));

    // === Test __atomic_reduce_add with vector<BFloat16, 2> ===
    // Each thread adds (0.03125, 0.015625), result should be (2.0, 1.0)
    // 64 * 0.03125 = 2.0, 64 * 0.015625 = 1.0
    vector<BFloat16, 2> bf16x2Val = vector<BFloat16, 2>(BFloat16(0.03125f), BFloat16(0.015625f));
    __atomic_reduce_add(bf16x2AddTarget[0], bf16x2Val);

    AllMemoryBarrierWithGroupSync();

    // Only thread 0 writes results
    if (tid == 0)
    {
        outputBuffer[0] = int(float(bf16AddTarget[0]));       // Expected: 4 (from 4.0bf)
        outputBuffer[1] = int(float(bf16x2AddTarget[0].x));   // Expected: 2 (from 2.0bf)
        outputBuffer[2] = int(float(bf16x2AddTarget[0].y));   // Expected: 1 (from 1.0bf)
    }
}

// BUF:      4
// BUF-NEXT: 2
// BUF-NEXT: 1
