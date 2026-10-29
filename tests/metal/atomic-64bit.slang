//TEST:SIMPLE(filecheck=METAL): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=METAL): -target metal -stage compute -entry computeMain -g
//TEST:SIMPLE(filecheck=NOFETCH): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage compute -entry computeMain

// MSL provides 64-bit integer atomics only as the void `atomic_min_explicit` and
// `atomic_max_explicit` on a device `atomic_ulong` (relaxed order only), and as the void
// `atomic_min`/`atomic_max` members of a `ulong` texture. A `uint64_t` min or max whose original
// value is unused lowers to those forms instead of the `atomic_fetch_*` calls that the Metal
// compiler rejects (shader-slang/slang#14031). The unsupported shapes are covered by
// atomic-64bit-unsupported.slang.

//METALLIB: @computeMain

// NOFETCH: void computeMain(
// NOFETCH-NOT: atomic_fetch_
// NOFETCH-NOT: = atomic_

RWStructuredBuffer<uint64_t> buffer;
RWStructuredBuffer<Atomic<uint64_t>> atomicBuffer;
RWTexture2D<uint64_t> texture;
RWTexture3D<uint64_t> texture3D;
RWTexture2DArray<uint64_t> textureArray;
uniform uint64_t* pointer;

[numthreads(64, 1, 1)]
void computeMain(uint tid : SV_DispatchThreadID)
{
    uint64_t key = (uint64_t(tid) << 32) | tid;
    uint64_t unusedOriginal;

    // METAL: atomic_min_explicit(((atomic_ulong device*)({{.*}})), key_{{[0-9]+}}, memory_order_relaxed);
    InterlockedMin(buffer[tid & 15], key);
    // METAL: atomic_max_explicit(((atomic_ulong device*)({{.*}})), key_{{[0-9]+}}, memory_order_relaxed);
    InterlockedMax(buffer[tid & 15], key);
    // METAL: atomic_min_explicit(((atomic_ulong device*)({{.*}})), key_{{[0-9]+}}, memory_order_relaxed);
    InterlockedMin(buffer[tid & 7], key, unusedOriginal);

    // METAL: atomic_min_explicit({{.*}}, key_{{[0-9]+}}, memory_order_relaxed);
    atomicBuffer[tid & 15].min(key);
    // METAL: atomic_max_explicit({{.*}}, key_{{[0-9]+}}, memory_order_relaxed);
    atomicBuffer[tid & 15].max(key);
    // METAL: atomic_min_explicit({{.*}}, key_{{[0-9]+}}, memory_order_relaxed);
    atomicBuffer[tid & 15].reduceMin(key);
    // METAL: atomic_max_explicit({{.*}}, key_{{[0-9]+}}, memory_order_relaxed);
    atomicBuffer[tid & 15].reduceMax(key);

    // METAL: .atomic_min({{.*}}, vec<ulong, 4>(key_{{[0-9]+}}));
    InterlockedMin(texture[uint2(tid, 0)], key);
    // METAL: .atomic_max({{.*}}, vec<ulong, 4>(key_{{[0-9]+}}));
    InterlockedMax(texture[uint2(tid, 0)], key);
    // METAL: .atomic_min({{.*}}, vec<ulong, 4>(key_{{[0-9]+}}));
    InterlockedMin(texture3D[uint3(tid, 0, 0)], key);
    // METAL: .atomic_max({{.*}}.xy, {{.*}}.z, vec<ulong, 4>(key_{{[0-9]+}}));
    InterlockedMax(textureArray[uint3(tid, 0, 1)], key);

    // METAL: atomic_min_explicit(((atomic_ulong device*)({{.*}})), key_{{[0-9]+}}, memory_order_relaxed);
    InterlockedMin(*pointer, key);
}
