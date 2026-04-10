// atomic-reduce-methods.slang
// Tests for Atomic<T>.reduce* methods (integer types) added to core.meta.slang.
// These methods are wrappers that:
//   - On CUDA: emit PTX `red` instructions (no old value returned, faster).
//   - On other targets: fall back to the existing Atomic<T> add/sub/min/max/etc.
//
// Tests integer methods: reduceAdd, reduceSub, reduceMax, reduceMin,
//                        reduceAnd, reduceOr, reduceXor, reduceInc, reduceDec.
// Floating-point reduceAdd is tested separately in atomic-reduce-methods-float.slang
// because HLSL does not support floating-point atomics.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-cuda -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-slang -compute -dx12 -profile cs_6_0 -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -emit-spirv-directly -compute -shaderobj -output-using-type -render-feature hardware-device
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-metal -compute -shaderobj -output-using-type

// Arithmetic reduction targets (IArithmeticAtomicable)
//TEST_INPUT:ubuffer(data=[0], stride=4):name=uintAddTarget
RWStructuredBuffer<Atomic<uint> > uintAddTarget;
//TEST_INPUT:ubuffer(data=[0], stride=4):name=intAddTarget
RWStructuredBuffer<Atomic<int> > intAddTarget;

//TEST_INPUT:ubuffer(data=[100], stride=4):name=uintSubTarget
RWStructuredBuffer<Atomic<uint> > uintSubTarget;
//TEST_INPUT:ubuffer(data=[100], stride=4):name=intSubTarget
RWStructuredBuffer<Atomic<int> > intSubTarget;

//TEST_INPUT:ubuffer(data=[1000], stride=4):name=uintMinTarget
RWStructuredBuffer<Atomic<uint> > uintMinTarget;
//TEST_INPUT:ubuffer(data=[0], stride=4):name=uintMaxTarget
RWStructuredBuffer<Atomic<uint> > uintMaxTarget;
//TEST_INPUT:ubuffer(data=[1000], stride=4):name=intMinTarget
RWStructuredBuffer<Atomic<int> > intMinTarget;
//TEST_INPUT:ubuffer(data=[-1000], stride=4):name=intMaxTarget
RWStructuredBuffer<Atomic<int> > intMaxTarget;

// Bitwise reduction targets (IBitAtomicable)
//TEST_INPUT:ubuffer(data=[0xFFFFFFFF], stride=4):name=uintAndTarget
RWStructuredBuffer<Atomic<uint> > uintAndTarget;
//TEST_INPUT:ubuffer(data=[0], stride=4):name=uintOrTarget
RWStructuredBuffer<Atomic<uint> > uintOrTarget;
//TEST_INPUT:ubuffer(data=[0], stride=4):name=uintXorTarget
RWStructuredBuffer<Atomic<uint> > uintXorTarget;

// Inc/Dec targets (IBitAtomicable)
//TEST_INPUT:ubuffer(data=[0], stride=4):name=uintIncTarget
RWStructuredBuffer<Atomic<uint> > uintIncTarget;
//TEST_INPUT:ubuffer(data=[100], stride=4):name=intDecTarget
RWStructuredBuffer<Atomic<int> > intDecTarget;

//TEST_INPUT: ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

static const int NUM_THREADS = 64;

[numthreads(NUM_THREADS, 1, 1)]
[shader("compute")]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;

    // === Test reduceAdd (IArithmeticAtomicable) ===
    // 64 threads each add 1 -> result 64
    uintAddTarget[0].reduceAdd(uint(1));
    intAddTarget[0].reduceAdd(1);

    // === Test reduceSub (IArithmeticAtomicable) ===
    // 64 threads each subtract 1 from 100 -> result 36
    uintSubTarget[0].reduceSub(uint(1));
    intSubTarget[0].reduceSub(1);

    // === Test reduceMin (IArithmeticAtomicable) ===
    // Each thread tries min with its tid; result = 0
    uintMinTarget[0].reduceMin(uint(tid));
    intMinTarget[0].reduceMin(int(tid));

    // === Test reduceMax (IArithmeticAtomicable) ===
    // Each thread tries max with its tid; result = 63
    uintMaxTarget[0].reduceMax(uint(tid));
    intMaxTarget[0].reduceMax(int(tid));

    // === Test reduceAnd (IBitAtomicable) ===
    // Each thread ANDs with ~(1 << (tid % 32)); result = 0
    uintAndTarget[0].reduceAnd(~(1u << (tid % 32)));

    // === Test reduceOr (IBitAtomicable) ===
    // Each thread ORs with (1 << (tid % 32)); result = 0xFFFFFFFF
    uintOrTarget[0].reduceOr(1u << (tid % 32));

    // === Test reduceXor (IBitAtomicable) ===
    // Threads 0-31 set bits, threads 32-63 clear them; result = 0
    uintXorTarget[0].reduceXor(1u << (tid % 32));

    // === Test reduceInc (IBitAtomicable) ===
    // 64 increments -> result 64
    uintIncTarget[0].reduceInc();

    // === Test reduceDec (IBitAtomicable) ===
    // 64 decrements from 100 -> result 36
    intDecTarget[0].reduceDec();

    AllMemoryBarrierWithGroupSync();

    // Only thread 0 writes results
    if (tid == 0)
    {
        int idx = 0;
        outputBuffer[idx++] = int(uintAddTarget[0].load());   // Expected: 64
        outputBuffer[idx++] = intAddTarget[0].load();          // Expected: 64
        outputBuffer[idx++] = int(uintSubTarget[0].load());    // Expected: 36 (100 - 64)
        outputBuffer[idx++] = intSubTarget[0].load();          // Expected: 36 (100 - 64)
        outputBuffer[idx++] = int(uintMinTarget[0].load());    // Expected: 0
        outputBuffer[idx++] = intMinTarget[0].load();          // Expected: 0
        outputBuffer[idx++] = int(uintMaxTarget[0].load());    // Expected: 63
        outputBuffer[idx++] = intMaxTarget[0].load();          // Expected: 63
        outputBuffer[idx++] = int(uintAndTarget[0].load());    // Expected: 0
        outputBuffer[idx++] = int(uintOrTarget[0].load());     // Expected: -1 (0xFFFFFFFF as signed)
        outputBuffer[idx++] = int(uintXorTarget[0].load());    // Expected: 0
        outputBuffer[idx++] = int(uintIncTarget[0].load());    // Expected: 64
        outputBuffer[idx++] = intDecTarget[0].load();          // Expected: 36
    }
}

// BUF:      64
// BUF-NEXT: 64
// BUF-NEXT: 36
// BUF-NEXT: 36
// BUF-NEXT: 0
// BUF-NEXT: 0
// BUF-NEXT: 63
// BUF-NEXT: 63
// BUF-NEXT: 0
// BUF-NEXT: -1
// BUF-NEXT: 0
// BUF-NEXT: 64
// BUF-NEXT: 36
