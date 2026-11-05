// WGSL's atomicCompareExchangeWeak may fail spuriously (exchanged == false although old_value equals
// the comparand). InterlockedCompareExchange / Atomic<T>.compareExchange callers detect success by
// comparing the old value with the comparand, so the WGSL lowering retries until the exchange
// happened or the value really differs. The pointer, comparand and value are evaluated once, before
// the loop, so every retry uses the same operands.
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -stage compute -entry computeMain
// Validate the emitted WGSL through Tint (ignored where the downstream compiler is absent).
//TEST:SIMPLE(filecheck=SPV): -target wgsl-spirv-asm -stage compute -entry computeMain

RWStructuredBuffer<Atomic<uint>> locks;
RWStructuredBuffer<uint> outputBuffer;
groupshared Atomic<int> groupLock;

// SPV: OpEntryPoint
// WGSL: let [[CAS:[A-Za-z0-9_]+_cas]]_ptr = &(locks_{{[0-9]+}}[{{.*}}]);
// WGSL-NEXT: let [[CAS]]_cmp = u32(0);
// WGSL-NEXT: let [[CAS]]_val = u32(1);
// WGSL-NEXT: var [[CAS]] = atomicCompareExchangeWeak([[CAS]]_ptr, [[CAS]]_cmp, [[CAS]]_val);
// WGSL-NEXT: loop
// WGSL-NEXT: {
// WGSL-NEXT: if ([[CAS]].exchanged || [[CAS]].old_value != [[CAS]]_cmp)
// WGSL: [[CAS]] = atomicCompareExchangeWeak([[CAS]]_ptr, [[CAS]]_cmp, [[CAS]]_val);
// WGSL: = [[CAS]].old_value;

// Operands read from memory are read once, before the loop.
// WGSL: let [[CAS2:[A-Za-z0-9_]+_cas]]_ptr = &(locks_{{[0-9]+}}[{{.*}}]);
// WGSL-NEXT: let [[CAS2]]_cmp = {{.*}}outputBuffer_{{[0-9]+}}[{{.*}}];
// WGSL-NEXT: let [[CAS2]]_val = {{.*}}outputBuffer_{{[0-9]+}}[{{.*}}];
// WGSL-NEXT: var [[CAS2]] = atomicCompareExchangeWeak([[CAS2]]_ptr, [[CAS2]]_cmp, [[CAS2]]_val);
// WGSL-NOT: outputBuffer
// WGSL: = [[CAS2]].old_value;

// WGSL: let [[CAS3:[A-Za-z0-9_]+_cas]]_ptr = &(groupLock_{{[0-9]+}});
// WGSL-NEXT: let [[CAS3]]_cmp = i32(0);
// WGSL-NEXT: let [[CAS3]]_val = i32(-1);
[numthreads(64, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    uint old = locks[tid.x % 4].compareExchange(0, 1);
    uint old2 = locks[(tid.x + 1) % 4].compareExchange(outputBuffer[0], outputBuffer[1]);
    int g = groupLock.compareExchange(0, -1);
    outputBuffer[tid.x] = old + old2 + uint(g);
}
