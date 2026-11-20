// `Atomic<T>.compareExchange` takes a success and a failure memory order. The CPU prelude helper
// takes one order and derives the order of a failed exchange from it, so the C++ emitter
// strengthens the order it passes until the derived failure order is at least the requested one.
// The last argument is the order, as the `MemoryOrder` values (0 relaxed, 1 acquire, 2 release,
// 3 acquire-release, 4 sequentially consistent).
//TEST:SIMPLE(filecheck=CPP):-target cpp -stage compute -entry computeMain

RWStructuredBuffer<Atomic<uint>> a;
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // CPP: _slang_atomic_compare_exchange_u32({{.*}}, 0);
    outputBuffer[0] = a[0].compareExchange(1u, 2u);
    // CPP: _slang_atomic_compare_exchange_u32({{.*}}, 1);
    outputBuffer[1] = a[0].compareExchange(1u, 2u, MemoryOrder.Relaxed, MemoryOrder.Acquire);
    // CPP: _slang_atomic_compare_exchange_u32({{.*}}, 3);
    outputBuffer[2] = a[0].compareExchange(1u, 2u, MemoryOrder.Release, MemoryOrder.Acquire);
    // CPP: _slang_atomic_compare_exchange_u32({{.*}}, 4);
    outputBuffer[3] = a[0].compareExchange(1u, 2u, MemoryOrder.Release, MemoryOrder.SeqCst);
    // A failure order weaker than the success order keeps the success order.
    // CPP: _slang_atomic_compare_exchange_u32({{.*}}, 3);
    outputBuffer[4] =
        a[0].compareExchange(1u, 2u, MemoryOrder.AcquireRelease, MemoryOrder.Relaxed);
}
