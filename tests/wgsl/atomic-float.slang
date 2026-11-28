// WGSL has no atomic<f32>. An Atomic<float> is an atomic<u32> holding the float's bits: loads,
// stores and exchanges bitcast, and add/sub/min/max are a compare-exchange loop.
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -stage compute -entry computeMain
// Validate the emitted WGSL through Tint (ignored where the downstream compiler is absent).
//TEST:SIMPLE(filecheck=SPV): -target wgsl-spirv-asm -stage compute -entry computeMain

RWStructuredBuffer<Atomic<float>> sums;
RWStructuredBuffer<float> outputBuffer;
groupshared Atomic<float> groupSum;

// SPV: OpEntryPoint
// WGSL-NOT: atomic<f32>
// WGSL-DAG: var<storage, read_write> sums_{{[0-9]+}} : array<atomic<u32>>;
// WGSL-DAG: var<workgroup> groupSum_{{[0-9]+}} : atomic<u32>;
// WGSL-DAG: atomicCompareExchangeWeak(&({{.*}}sums_{{[0-9]+}}{{.*}}), {{.*}}_old, bitcast<u32>(bitcast<f32>({{.*}}_old) + (
// WGSL-DAG: atomicCompareExchangeWeak(&({{.*}}sums_{{[0-9]+}}{{.*}}), {{.*}}_old, bitcast<u32>(bitcast<f32>({{.*}}_old) - (
// WGSL-DAG: bitcast<u32>(min(bitcast<f32>(
// WGSL-DAG: bitcast<u32>(max(bitcast<f32>(
// WGSL-DAG: bitcast<f32>(atomicExchange(&(
// WGSL-DAG: bitcast<f32>(atomicLoad(&(
// WGSL-DAG: atomicStore(&({{.*}}), bitcast<u32>(
// A strong compare exchange: retried while the weak one fails spuriously.
// WGSL-DAG: if ({{.*}}_cas.exchanged || {{.*}}_cas.old_value != bitcast<u32>(0.0f))
// WGSL-DAG: atomicCompareExchangeWeak(&(groupSum_{{[0-9]+}}), {{.*}}_old, bitcast<u32>(bitcast<f32>({{.*}}_old) + (
[numthreads(64, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID, uint gi: SV_GroupIndex)
{
    if (gi == 0)
        groupSum.store(0.0);
    GroupMemoryBarrierWithGroupSync();

    float v = float(tid.x) * 0.5;
    float a = sums[0].add(v);
    float b = sums[1].sub(v);
    float c = sums[2].min(v);
    float d = sums[3].max(v);
    float e = sums[4].exchange(v);
    float f = sums[5].compareExchange(0.0, v);
    groupSum.add(1.0);
    GroupMemoryBarrierWithGroupSync();

    outputBuffer[tid.x] = a + b + c + d + e + f + groupSum.load() + sums[6].load();
    if (tid.x == 0)
        sums[7].store(v);
}
