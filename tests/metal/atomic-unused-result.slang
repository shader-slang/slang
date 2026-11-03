// Atomic operations whose result is discarded are emitted on Metal as bare calls
// (shader-slang/slang#14058). Binding the result to a temporary that is never read draws
// -Wunused-variable from the Metal compiler, and a discarded texture atomic ending in `.x` draws
// -Wunused-value. A result that is used is still bound to a temporary.

//TEST:SIMPLE(filecheck=REPRO):-target metal -entry reproMain -stage compute
//TEST:SIMPLE(filecheck=REPRO):-target metal -entry reproMain -stage compute -g
//TEST:SIMPLE(filecheck=UNUSED):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=UNUSED):-target metal -entry computeMain -stage compute -g
//TEST:SIMPLE(filecheck=CHECK):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK):-target metal -entry computeMain -stage compute -g
//TEST:SIMPLE(filecheck=LIB):-target metallib -entry computeMain -stage compute -Xmetal -Werror=unused-variable -Xmetal -Werror=unused-value
//TEST:SIMPLE(filecheck=LIBREPRO):-target metallib -entry reproMain -stage compute -Xmetal -Werror=unused-variable -Xmetal -Werror=unused-value
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -shaderobj -output-using-type
//TEST(compute, metal):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-metal -compute -shaderobj -output-using-type

// The kernel from the report in shader-slang/slang#14058.
[shader("compute")]
[numthreads(64, 1, 1)]
void reproMain(uniform uint* counts, uniform Atomic<float>* sums, uint3 t: SV_DispatchThreadID)
{
    // REPRO-NOT: {{= .*atomic_fetch}}
    // REPRO: {{^ *}}atomic_fetch_add_explicit({{.*}}counts_0
    InterlockedAdd(counts[t.x % 7u], 1u);
    // REPRO-NOT: {{= .*atomic_fetch}}
    // REPRO: {{^ *}}atomic_fetch_add_explicit({{.*}}sums_0
    // REPRO-NOT: {{= .*atomic_fetch}}
    sums[t.x % 5u].add(1.5f, MemoryOrder.Relaxed);
}

//TEST_INPUT: set results = out ubuffer(data=[0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4)
RWStructuredBuffer<uint> results;
//TEST_INPUT: set atomics = ubuffer(data=[0 0 10 0 0], stride=4)
RWStructuredBuffer<Atomic<uint>> atomics;
//TEST_INPUT: set tex = RWTexture2D(format=R32Uint, size=4, content=one)
RWTexture2D<uint> tex;

groupshared uint counter;

[numthreads(4, 1, 1)]
void computeMain(uint tid: SV_GroupIndex)
{
    if (tid == 0)
        counter = 0;
    GroupMemoryBarrierWithGroupSync();

    // Every result in this block is discarded, so no temporary is bound to any of them. The
    // `UNUSED` checks hold from the start of the output up to the first used result below.
    // UNUSED-NOT: {{= .*atomic_(fetch|exchange|load|compare)}}
    // UNUSED-NOT: ).x;
    // UNUSED-NOT: = _metal_cas_comparand

    // CHECK: {{^ *}}atomic_fetch_add_explicit({{.*}}results_0+int(0)
    InterlockedAdd(results[0], 1u);
    // CHECK: {{^ *}}atomic_fetch_max_explicit({{.*}}results_0+int(1)
    InterlockedMax(results[1], tid + 10u);
    // CHECK-NOT: {{^ *uint [A-Za-z_][A-Za-z0-9_]*;$}}
    // CHECK: {{^ *}}atomic_compare_exchange_weak_explicit({{.*}}results_0+int(2)
    InterlockedCompareStore(results[2], 0u, 7u);
    // CHECK: {{^ *}}atomic_fetch_add_explicit({{.*}}counter_0
    InterlockedAdd(counter, 2u);
    // CHECK: {{^ *}}({{.*}}tex_0).atomic_fetch_add({{.*}}(1U));{{$}}
    InterlockedAdd(tex[uint2(tid, 0)], 1u);
    // CHECK-NOT: {{^ *uint [A-Za-z_][A-Za-z0-9_]*;$}}
    // CHECK: {{^ *}}({{.*}}tex_0).atomic_compare_exchange_weak({{.*}}(3U));{{$}}
    InterlockedCompareStore(tex[uint2(tid, 2)], 1u, 3u);
    // CHECK: {{^ *}}atomic_fetch_add_explicit({{.*}}atomics_0+int(0), 2U
    atomics[0].add(2u);
    // CHECK: {{^ *}}atomic_fetch_add_explicit({{.*}}atomics_0+int(1), 1,
    atomics[1].increment();
    // CHECK: {{^ *}}atomic_fetch_sub_explicit({{.*}}atomics_0+int(2), 1,
    atomics[2].decrement();
    // CHECK: {{^ *}}atomic_exchange_explicit({{.*}}atomics_0+int(3)
    atomics[3].exchange(5u);
    // CHECK-NOT: {{^ *uint [A-Za-z_][A-Za-z0-9_]*;$}}
    // CHECK: {{^ *}}atomic_compare_exchange_weak_explicit({{.*}}atomics_0+int(4)
    atomics[4].compareExchange(0u, 9u);
    // CHECK: {{^ *}}atomic_load_explicit({{.*}}atomics_0+int(0)
    atomics[0].load();
    // With `-g`, each never-read out parameter below keeps a debug-only use of its atomic's result.
    // CHECK: {{^ *}}atomic_fetch_add_explicit({{.*}}results_0+int(12)
    uint unread;
    InterlockedAdd(results[12], 1u, unread);
    // CHECK-NOT: {{^ *uint [A-Za-z_][A-Za-z0-9_]*;$}}
    // CHECK: {{^ *}}atomic_compare_exchange_weak_explicit({{.*}}results_0+int(15)
    uint casUnread;
    InterlockedCompareExchange(results[15], 0u, 2u, casUnread);
    // CHECK: {{^ *}}({{.*}}tex_0).atomic_exchange({{.*}}(5U));{{$}}
    uint texUnread;
    InterlockedExchange(tex[uint2(tid, 3)], 5u, texUnread);

    if (tid == 0)
    {
        // UNUSED: original_0 = atomic_fetch_add_explicit(
        // CHECK: original_0 = atomic_fetch_add_explicit(
        uint original;
        InterlockedAdd(results[4], 5u, original);
        results[5] = original + 100u;
        // CHECK: texOriginal_0 = {{.*}}tex_0).atomic_fetch_add({{.*}}).x;
        uint texOriginal;
        InterlockedAdd(tex[uint2(0, 1)], 1u, texOriginal);
        results[6] = texOriginal;
        // CHECK: casOriginal_0 = _metal_cas_comparand;
        uint casOriginal;
        InterlockedCompareExchange(results[13], 0u, 1u, casOriginal);
        results[14] = casOriginal;
    }

    AllMemoryBarrierWithGroupSync();
    if (tid == 0)
    {
        results[3] = counter;
        // CHECK: = atomic_load_explicit({{.*}}atomics_0+i_0
        for (uint i = 0; i < 5; i++)
            results[7 + i] = atomics[i].load();
        for (uint i = 0; i < 4; i++)
        {
            results[16 + i] = tex[uint2(i, 0)];
            results[20 + i] = tex[uint2(i, 2)];
            results[24 + i] = tex[uint2(i, 3)];
        }
        results[28] = tex[uint2(0, 1)];
    }
}

// LIB: @computeMain
// LIBREPRO: @reproMain

// Expected `results`, by index, from 4 threads: 0 and 12 count 4 adds of 1; 1 is the max of
// tid + 10; 2 and 15 are compare-stores of 7 and 2; 3 is `counter` after 4 adds of 2; 4 is the
// input 1 plus 5; 5 and 6 are the used results 1 + 100 and texel (0, 1) before its add; 7-11 are
// `atomics` after 4 adds of 2, 4 increments, 4 decrements from 10, an exchange of 5 and a
// compare-store of 9; 13 and 14 are a compare-exchange of 1 and its original 0; 16-19, 20-23 and
// 24-27 are texture rows 0 (1 + 1), 2 (compare-store of 3) and 3 (exchange of 5); 28 is texel
// (0, 1) after its add; 29-31 are not written.
// BUF:      4
// BUF-NEXT: 13
// BUF-NEXT: 7
// BUF-NEXT: 8
// BUF-NEXT: 6
// BUF-NEXT: 101
// BUF-NEXT: 1
// BUF-NEXT: 8
// BUF-NEXT: 4
// BUF-NEXT: 6
// BUF-NEXT: 5
// BUF-NEXT: 9
// BUF-NEXT: 4
// BUF-NEXT: 1
// BUF-NEXT: 0
// BUF-NEXT: 2
// BUF-NEXT: 2
// BUF-NEXT: 2
// BUF-NEXT: 2
// BUF-NEXT: 2
// BUF-NEXT: 3
// BUF-NEXT: 3
// BUF-NEXT: 3
// BUF-NEXT: 3
// BUF-NEXT: 5
// BUF-NEXT: 5
// BUF-NEXT: 5
// BUF-NEXT: 5
// BUF-NEXT: 2
// BUF-NEXT: 0
// BUF-NEXT: 0
// BUF-NEXT: 0
