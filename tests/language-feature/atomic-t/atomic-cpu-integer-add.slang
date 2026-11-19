// Regression test for https://github.com/shader-slang/slang/issues/14183.
//
// The C++ backend lowers integer `Atomic<T>.add` to the `_slang_atomic_add_*` prelude helpers, and
// `reduceAdd` and `+=` inline to the same operation. The other atomic operations are reported as
// unsupported on CPU targets (see tests/diagnostics/atomic-t-cpp-unsupported.slang); this test
// checks that the supported integer add still runs for each 32- and 64-bit integer type. The
// int64_t and uint64_t blocks cross 2^32, so a 64-bit helper that updated only the low word fails.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

//TEST_INPUT: set intBuffer = ubuffer(data=[10 0 0 0], stride=4)
RWStructuredBuffer<Atomic<int>> intBuffer;
//TEST_INPUT: set uintBuffer = ubuffer(data=[20 0 0 0], stride=4)
RWStructuredBuffer<Atomic<uint>> uintBuffer;
//TEST_INPUT: set int64Buffer = ubuffer(data=[0 0 0 0], stride=8)
RWStructuredBuffer<Atomic<int64_t>> int64Buffer;
//TEST_INPUT: set uint64Buffer = ubuffer(data=[0 0 0 0], stride=8)
RWStructuredBuffer<Atomic<uint64_t>> uint64Buffer;

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // Each `add` returns the value stored before it.
    outputBuffer[0] = intBuffer[0].add(1);
    intBuffer[0].reduceAdd(2);
    intBuffer[0] += 3;
    outputBuffer[1] = intBuffer[0].add(0);

    outputBuffer[2] = int(uintBuffer[0].add(1u));
    uintBuffer[0] += 2u;
    outputBuffer[3] = int(uintBuffer[0].add(0u));

    outputBuffer[4] = int(int64Buffer[0].add(5));
    int64Buffer[0] += 0x100000000;
    outputBuffer[5] = int(int64Buffer[0].add(0) >> 32);

    outputBuffer[6] = int(uint64Buffer[0].add(7));
    uint64Buffer[0].reduceAdd(1);
    outputBuffer[7] = int(uint64Buffer[0].add(0));
    uint64Buffer[0] += 0x100000000ULL;
    outputBuffer[8] = int(uint64Buffer[0].add(0) >> 32);
}

// CHECK: type: int32_t
// CHECK-NEXT: 10
// CHECK-NEXT: 16
// CHECK-NEXT: 20
// CHECK-NEXT: 23
// CHECK-NEXT: 0
// CHECK-NEXT: 1
// CHECK-NEXT: 0
// CHECK-NEXT: 8
// CHECK-NEXT: 1
