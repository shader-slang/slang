// MSL only has a weak compare-exchange, which may fail spuriously: it returns false and leaves the
// expected value equal to the comparand although memory held it. InterlockedCompareExchange and
// Atomic<T>.compareExchange callers detect success by comparing the old value with the comparand,
// so the Metal lowering retries until the exchange happened or the value really differs (#14054).
// The retry test compares bit patterns, so a float -0.0 in memory does not pass as a +0.0
// comparand and a NaN comparand still retries. The pointer, comparand and value are evaluated once,
// before the loop.

//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=LIB): -target metallib -stage compute -entry computeMain
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-slang -compute -mtl -shaderobj -output-using-type

// LIB: @computeMain

//TEST_INPUT:ubuffer(data=[-1 -1 -1 -1], stride=4):name=slots
RWStructuredBuffer<int> slots;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):name=locks
RWStructuredBuffer<Atomic<uint>> locks;

//TEST_INPUT:ubuffer(data=[0.0 0.0], stride=4):name=floats
RWStructuredBuffer<Atomic<float>> floats;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

groupshared Atomic<int> groupLock;

// InterlockedCompareExchange on a plain int buffer.
// CHECK: int [[OLD:[A-Za-z0-9_]+]];
// CHECK-NEXT: {
// CHECK-NEXT: auto _metal_cas_ptr = ((atomic_int device*)({{.*}}slots_{{[0-9]+}}+
// CHECK-NEXT: int _metal_cas_comparand = int(-1);
// CHECK-NEXT: int _metal_cas_value = {{.*}};
// CHECK-NEXT: int _metal_cas_expected = _metal_cas_comparand;
// CHECK-NEXT: while (!atomic_compare_exchange_weak_explicit(_metal_cas_ptr, &_metal_cas_expected, _metal_cas_value, memory_order_relaxed, memory_order_relaxed) && _metal_cas_expected == _metal_cas_comparand)
// CHECK: [[OLD]] = _metal_cas_expected;

// Atomic<uint>.compareExchange: the comparand and value are read from memory once, before the loop.
// CHECK: auto _metal_cas_ptr = {{.*}}locks_{{[0-9]+}}
// CHECK-NEXT: uint _metal_cas_comparand = {{.*}}outputBuffer_{{[0-9]+}}
// CHECK-NEXT: uint _metal_cas_value = {{.*}}outputBuffer_{{[0-9]+}}
// CHECK-NEXT: uint _metal_cas_expected = _metal_cas_comparand;
// CHECK-NEXT: while (!atomic_compare_exchange_weak_explicit(_metal_cas_ptr, &_metal_cas_expected, _metal_cas_value,
// CHECK-NOT: outputBuffer
// CHECK: = _metal_cas_expected;

// Atomic<float>.compareExchange compares bit patterns.
// CHECK: float _metal_cas_expected = _metal_cas_comparand;
// CHECK-NEXT: while (!atomic_compare_exchange_weak_explicit(_metal_cas_ptr, &_metal_cas_expected, _metal_cas_value, {{.*}}) && as_type<uint>(_metal_cas_expected) == as_type<uint>(_metal_cas_comparand))

// groupshared Atomic<int>.
// CHECK: auto _metal_cas_ptr = {{.*}}groupLock
// CHECK: while (!atomic_compare_exchange_weak_explicit(_metal_cas_ptr,

[numthreads(64, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID, uint gi: SV_GroupIndex)
{
    if (gi == 0)
        groupLock.store(0);
    GroupMemoryBarrierWithGroupSync();

    // Exactly one thread claims each of the 4 slots, and every other thread sees the winner.
    int old;
    InterlockedCompareExchange(slots[tid.x % 4], -1, int(tid.x), old);
    bool claimedSlot = (old == -1);

    // The comparand (0) and the value (1) come from memory.
    uint lockOld = locks[tid.x % 4].compareExchange(outputBuffer[5], outputBuffer[5] + 1);
    bool claimedLock = (lockOld == 0);

    // -0.0 in memory does not match a +0.0 comparand: only floats[1] (+0.0) is exchanged.
    if (tid.x == 0)
        floats[0].store(-0.0f);
    AllMemoryBarrierWithGroupSync();
    float f0 = floats[0].compareExchange(0.0f, 1.0f);
    float f1 = floats[1].compareExchange(0.0f, 1.0f);

    int g = groupLock.compareExchange(0, int(gi) + 1);
    bool claimedGroup = (g == 0);

    AllMemoryBarrierWithGroupSync();
    InterlockedAdd(outputBuffer[0], claimedSlot ? 1u : 0u);
    InterlockedAdd(outputBuffer[1], claimedLock ? 1u : 0u);
    InterlockedAdd(outputBuffer[2], claimedGroup ? 1u : 0u);
    if (tid.x == 0)
    {
        outputBuffer[3] = asuint(floats[0].load());
        outputBuffer[4] = asuint(f0) == 0x80000000u && asuint(floats[1].load()) == asuint(1.0f) ? 1u : 0u;
    }
}

// BUF: 4
// BUF-NEXT: 4
// BUF-NEXT: 1
// BUF-NEXT: 2147483648
// BUF-NEXT: 1
// BUF-NEXT: 0
