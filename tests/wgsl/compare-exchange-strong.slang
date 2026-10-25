// WGSL's atomicCompareExchangeWeak may fail spuriously (exchanged == false although old_value equals
// the comparand). InterlockedCompareExchange / Atomic<T>.compareExchange callers detect success by
// comparing the old value with the comparand, so the WGSL lowering retries until the exchange
// happened or the value really differs.
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -stage compute -entry computeMain
// Validate the emitted WGSL through Tint (ignored where the downstream compiler is absent).
//TEST:SIMPLE(filecheck=SPV): -target wgsl-spirv-asm -stage compute -entry computeMain

RWStructuredBuffer<Atomic<uint>> locks;
RWStructuredBuffer<uint> outputBuffer;
groupshared Atomic<int> groupLock;

// SPV: OpEntryPoint
// WGSL: var [[CAS:[A-Za-z0-9_]+_cas]] = atomicCompareExchangeWeak(&(locks_{{[0-9]+}}[{{.*}}]), u32(0), u32(1));
// WGSL-NEXT: loop
// WGSL-NEXT: {
// WGSL-NEXT: if ([[CAS]].exchanged || [[CAS]].old_value != (u32(0)))
// WGSL: [[CAS]] = atomicCompareExchangeWeak(&(locks_{{[0-9]+}}[{{.*}}]), u32(0), u32(1));
// WGSL: = [[CAS]].old_value;
// WGSL: atomicCompareExchangeWeak(&(groupLock_{{[0-9]+}}), i32(0), i32(-1))
[numthreads(64, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    uint old = locks[tid.x % 4].compareExchange(0, 1);
    int g = groupLock.compareExchange(0, -1);
    outputBuffer[tid.x] = old + uint(g);
}
