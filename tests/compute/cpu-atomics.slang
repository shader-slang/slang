// Atomics on the CPU targets: `Interlocked*` on buffer elements and `groupshared` memory, and
// `Atomic<T>`, for the operations and element types the C++ prelude implements. Each output records
// the prior value an operation returned, or the final value. (Float min/max are left out here: the
// CUDA lane that slang-test synthesizes from this test would hit #14026; the unit test
// cpuAtomicsAreThreadSafe covers them on the CPU.)

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-cpu -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=CPP):-target cpp -stage compute -entry computeMain

//TEST_INPUT:ubuffer(data=[10 20 30 40 50 60 70 80], stride=4):name=u
RWStructuredBuffer<uint> u;
//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):name=f
RWStructuredBuffer<float> f;
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):name=au
RWStructuredBuffer<Atomic<uint>> au;
//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):name=af
RWStructuredBuffer<Atomic<float>> af;
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=8):name=a64
RWStructuredBuffer<Atomic<int64_t>> a64;
//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):name=ai
RWStructuredBuffer<Atomic<int>> ai;
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

groupshared uint shared;

[numthreads(1, 1, 1)]
void computeMain()
{
    uint o;
    // CPP-DAG: _slang_atomic_add_u32(
    InterlockedAdd(u[0], 5u, o);
    outputBuffer[0] = o;
    // CPP-DAG: _slang_atomic_and_u32(
    InterlockedAnd(u[1], 6u, o);
    outputBuffer[1] = o;
    // CPP-DAG: _slang_atomic_or_u32(
    InterlockedOr(u[2], 1u, o);
    outputBuffer[2] = o;
    // CPP-DAG: _slang_atomic_xor_u32(
    InterlockedXor(u[3], 0xffu, o);
    outputBuffer[3] = o;
    // CPP-DAG: _slang_atomic_min_u32(
    InterlockedMin(u[4], 7u, o);
    outputBuffer[4] = o;
    // CPP-DAG: _slang_atomic_max_u32(
    InterlockedMax(u[5], 99u, o);
    outputBuffer[5] = o;
    // CPP-DAG: _slang_atomic_exchange_u32(
    InterlockedExchange(u[6], 3u, o);
    outputBuffer[6] = o;
    // CPP-DAG: _slang_atomic_compare_exchange_u32(
    InterlockedCompareExchange(u[7], 80u, 81u, o);
    outputBuffer[7] = o;

    // BUF: 10
    // BUF-NEXT: 20
    // BUF-NEXT: 30
    // BUF-NEXT: 40
    // BUF-NEXT: 50
    // BUF-NEXT: 60
    // BUF-NEXT: 70
    // BUF-NEXT: 80

    // CPP-DAG: _slang_atomic_add_f32(
    float fo;
    InterlockedAdd(f[0], 1.5f, fo);
    InterlockedAdd(f[0], 2.25f);
    outputBuffer[8] = asuint(fo);
    outputBuffer[9] = uint(f[0] * 4.0f);
    // BUF-NEXT: 0
    // BUF-NEXT: 15

    shared = 0;
    InterlockedAdd(shared, 4u);
    InterlockedMax(shared, 9u, o);
    outputBuffer[10] = o * 100 + shared;
    // BUF-NEXT: 409

    // CPP-DAG: _slang_atomic_load_u32(
    // CPP-DAG: _slang_atomic_store_u32(
    au[0].store(11u);
    outputBuffer[11] = au[0].load();
    outputBuffer[12] = au[0].add(1u) * 100 + au[0].sub(2u);
    outputBuffer[13] = au[1].increment() * 100 + au[1].decrement();
    outputBuffer[14] = au[2].or(12u) * 100 + au[2].and(10u);
    outputBuffer[15] = au[2].xor(3u) * 100 + au[2].load();
    outputBuffer[16] = au[3].max(5u) * 100 + au[3].min(2u);
    outputBuffer[17] = au[3].exchange(7u) * 100 + au[3].compareExchange(7u, 8u);
    outputBuffer[18] = au[3].compareExchange(0u, 9u) * 100 + au[3].load();
    // BUF-NEXT: 11
    // BUF-NEXT: 1112
    // BUF-NEXT: 1
    // BUF-NEXT: 12
    // BUF-NEXT: 811
    // BUF-NEXT: 5
    // BUF-NEXT: 207
    // BUF-NEXT: 808

    // CPP-DAG: _slang_atomic_sub_f32(
    // CPP-DAG: _slang_atomic_compare_exchange_f32(
    af[0].store(1.0f);
    float p0 = af[0].add(0.5f);
    float p1 = af[0].sub(0.25f);
    float p2 = af[0].exchange(6.0f);
    float p3 = af[0].compareExchange(6.0f, 7.5f);
    outputBuffer[19] = uint(p0 * 4.0f);
    outputBuffer[20] = uint(p1 * 4.0f);
    outputBuffer[21] = uint(p2 * 4.0f);
    outputBuffer[22] = uint(p3 * 4.0f);
    outputBuffer[23] = uint(af[0].load() * 4.0f);
    // BUF-NEXT: 4
    // BUF-NEXT: 6
    // BUF-NEXT: 5
    // BUF-NEXT: 24
    // BUF-NEXT: 30

    // CPP-DAG: _slang_atomic_sub_i64(
    // CPP-DAG: _slang_atomic_min_i64(
    a64[0].store(-5ll);
    int64_t q0 = a64[0].sub(1ll << 40);
    int64_t q1 = a64[0].min(-(1ll << 41));
    int64_t q2 = a64[0].load();
    outputBuffer[24] = uint(q0 + 10);
    outputBuffer[25] = uint((q1 + 5) >> 32);
    outputBuffer[26] = uint((-q2) >> 32);
    // BUF-NEXT: 5
    // BUF-NEXT: 4294967040
    // BUF-NEXT: 512

    // Subtracting the most negative value wraps around.
    // CPP-DAG: _slang_atomic_sub_i32(
    ai[0].store(7);
    int r0 = ai[0].sub(int(0x80000000u));
    outputBuffer[27] = uint(r0) * 100 + uint(ai[0].load() - int(0x80000000u));
    a64[1].store(5ll);
    int64_t r1 = a64[1].sub(int64_t(1ull << 63));
    int64_t r2 = a64[1].load();
    outputBuffer[28] = uint(r1);
    outputBuffer[29] = uint(r2 >> 32);
    outputBuffer[30] = uint(r2);
    // BUF-NEXT: 707
    // BUF-NEXT: 5
    // BUF-NEXT: 2147483648
    // BUF-NEXT: 5
}
