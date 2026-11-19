//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain -minimum-slang-optimization
//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain -minimum-slang-optimization -g

// `-minimum-slang-optimization` skips `checkUnsupportedInst`, which reports the 64-bit integer
// atomics that MSL cannot express. The Metal emitter picks the void `atomic_min_explicit` and
// `atomic_max_explicit` forms with the same predicate as that check, so a supported 64-bit min or
// max is emitted the same way when the check does not run (see atomic-64bit.slang).

RWStructuredBuffer<uint64_t> buffer;
RWStructuredBuffer<Atomic<uint64_t>> atomicBuffer;

//CHECK: void computeMain(
//CHECK-NOT: atomic_fetch_
//CHECK: atomic_min_explicit(((atomic_ulong device*)({{.*}})), key_{{[0-9]+}}, memory_order_relaxed);
//CHECK-NOT: atomic_fetch_
//CHECK: atomic_max_explicit({{.*}}, key_{{[0-9]+}}, memory_order_relaxed);
//CHECK-NOT: atomic_fetch_

[numthreads(64, 1, 1)]
void computeMain(uint tid : SV_DispatchThreadID)
{
    uint64_t key = uint64_t(tid);
    InterlockedMin(buffer[tid & 15], key);
    atomicBuffer[tid & 15].max(key);
}
