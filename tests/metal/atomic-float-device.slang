// Metal supports `atomic_float` load, exchange and compare-exchange on device memory.
// Regression test for https://github.com/shader-slang/slang/issues/14060.

//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain -minimum-slang-optimization
//TEST:SIMPLE(filecheck=LIB): -target metallib -stage compute -entry computeMain

struct Args
{
    Atomic<float>* values;
    float* results;
}

// `pad` puts `value` at a nonzero offset inside the buffer element.
struct AtomicField
{
    uint pad;
    Atomic<float> value;
}

RWStructuredBuffer<float> floatBuffer;
RWStructuredBuffer<Atomic<float>> atomicFloatBuffer;
RWStructuredBuffer<AtomicField> structBuffer;

[numthreads(64, 1, 1)]
void computeMain(uniform Args* args, uint3 tid: SV_DispatchThreadID)
{
    uint i = tid.x;
    float result = 0.0f;

    // CHECK: atomic_load_explicit(
    result += args->values[i].load(MemoryOrder.Relaxed);
    // CHECK: atomic_exchange_explicit(
    result += args->values[i].exchange(2.0f, MemoryOrder.Relaxed);
    // CHECK: atomic_compare_exchange_weak_explicit(
    result += args->values[i].compareExchange(0.0f, 3.0f, MemoryOrder.Relaxed);

    // CHECK: atomic_load_explicit(
    result += atomicFloatBuffer[i].load();
    // CHECK: atomic_exchange_explicit(
    result += atomicFloatBuffer[i].exchange(4.0f);
    // CHECK: atomic_compare_exchange_weak_explicit(
    result += atomicFloatBuffer[i].compareExchange(0.0f, 5.0f);

    float original;
    // CHECK: atomic_exchange_explicit(((atomic_float device*)
    InterlockedExchange(floatBuffer[i], 6.0f, original);
    result += original;
    // CHECK: atomic_compare_exchange_weak_explicit(((atomic_float device*)
    InterlockedCompareExchange(floatBuffer[i], 0.0f, 7.0f, original);
    result += original;
    // CHECK: atomic_compare_exchange_weak_explicit(((atomic_float device*)
    InterlockedCompareExchangeFloatBitwise(floatBuffer[i], 0.0f, 8.0f, original);
    result += original;
    // CHECK: atomic_compare_exchange_weak_explicit(((atomic_float device*)
    InterlockedCompareStore(floatBuffer[i], 0.0f, 9.0f);
    // CHECK: atomic_compare_exchange_weak_explicit(((atomic_float device*)
    InterlockedCompareStoreFloatBitwise(floatBuffer[i], 0.0f, 10.0f);
    // CHECK: atomic_fetch_add_explicit(((atomic_float device*)
    InterlockedAdd(floatBuffer[i], 11.0f, original);
    result += original;

    // CHECK: atomic_load_explicit(
    result += structBuffer[i].value.load();

    args->results[i] = result;
}

// LIB: @computeMain
