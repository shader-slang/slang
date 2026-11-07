//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target metal -stage compute -entry computeMain
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target metal -stage compute -entry computeMain -g

// MSL has no spelling for these 64-bit integer atomics, so each one is diagnosed instead of
// being emitted as a call that the Metal compiler rejects (shader-slang/slang#14031). The
// supported forms are covered by atomic-64bit.slang.

RWStructuredBuffer<uint64_t> buffer;
RWStructuredBuffer<int64_t> signedBuffer;
RWStructuredBuffer<Atomic<uint64_t>> atomicBuffer;
RWStructuredBuffer<Atomic<int64_t>> signedAtomicBuffer;
RWTexture2D<uint64_t> texture;
groupshared uint64_t sharedMemory[16];

[numthreads(64, 1, 1)]
void computeMain(uint tid : SV_DispatchThreadID)
{
    uint64_t value = uint64_t(tid);
    uint64_t original[8];

    // Fetch-modify, exchange and compare-exchange on a device `atomic_ulong`.
    InterlockedAdd(buffer[0], value);
//CHECK:          ^ E55204
//CHECK:          ^ other than min or max
    InterlockedAnd(buffer[0], value);
//CHECK:          ^ E55204
//CHECK:          ^ other than min or max
    InterlockedOr(buffer[0], value);
//CHECK:         ^ E55204
//CHECK:         ^ other than min or max
    InterlockedXor(buffer[0], value);
//CHECK:          ^ E55204
//CHECK:          ^ other than min or max
    InterlockedExchange(buffer[0], value);
//CHECK:               ^ E55204
//CHECK:               ^ other than min or max
    InterlockedCompareExchange(buffer[0], 0ull, value, original[0]);
//CHECK:                      ^ E55204
//CHECK:                      ^ other than min or max
    InterlockedCompareStore(buffer[0], 0ull, value);
//CHECK:                   ^ E55204
//CHECK:                   ^ other than min or max

    // A min or max whose original value is used.
    InterlockedMin(buffer[1], value, original[1]);
//CHECK:          ^ E55204
//CHECK:          ^ whose original value is used
    InterlockedMax(buffer[1], value, original[2]);
//CHECK:          ^ E55204
//CHECK:          ^ whose original value is used

    // A signed 64-bit min or max: MSL has no `atomic_long`.
    InterlockedMin(signedBuffer[0], int64_t(tid));
//CHECK:          ^ E55204
//CHECK:          ^ signed integer atomic min or max
    signedAtomicBuffer[0].max(int64_t(tid));
//CHECK:                     ^ E55204
//CHECK:                     ^ signed integer atomic min or max

    // A min on threadgroup memory: the 64-bit MSL min and max take only a device pointer.
    InterlockedMin(sharedMemory[tid & 15], value);
//CHECK:          ^ E55204
//CHECK:          ^ outside device memory

    // `Atomic<uint64_t>` methods other than a relaxed min or max with an unused result.
    original[3] = atomicBuffer[0].load();
//CHECK:                              ^ E55204
//CHECK:                              ^ other than min or max
    atomicBuffer[0].store(value);
//CHECK:                 ^ E55204
//CHECK:                 ^ other than min or max
    original[4] = atomicBuffer[0].exchange(value);
//CHECK:                                  ^ E55204
//CHECK:                                  ^ other than min or max
    original[5] = atomicBuffer[0].compareExchange(0ull, value);
//CHECK:                                         ^ E55204
//CHECK:                                         ^ other than min or max
    atomicBuffer[0].add(value);
//CHECK:               ^ E55204
//CHECK:               ^ other than min or max
    atomicBuffer[0].sub(value);
//CHECK:               ^ E55204
//CHECK:               ^ other than min or max
    atomicBuffer[0].increment();
//CHECK:                     ^ E55204
//CHECK:                     ^ other than min or max
    atomicBuffer[0].decrement();
//CHECK:                     ^ E55204
//CHECK:                     ^ other than min or max
    atomicBuffer[0].min(value, MemoryOrder.SeqCst);
//CHECK:               ^ E55204
//CHECK:               ^ memory order other than relaxed

    // A `ulong` texture supports only the `atomic_min` and `atomic_max` members, which return void.
    InterlockedAdd(texture[uint2(tid, 0)], value);
//CHECK:          ^ E55204
//CHECK:          ^ other than min or max
    InterlockedExchange(texture[uint2(tid, 0)], value);
//CHECK:               ^ E55204
//CHECK:               ^ other than min or max
    InterlockedCompareExchange(texture[uint2(tid, 0)], 0ull, value, original[6]);
//CHECK:                      ^ E55204
//CHECK:                      ^ other than min or max
    InterlockedMax(texture[uint2(tid, 0)], value, original[7]);
//CHECK:          ^ E55204
//CHECK:          ^ whose original value is used

    for (int i = 0; i < 8; i++)
        buffer[2 + i] = original[i];
}
