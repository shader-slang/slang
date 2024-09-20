//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-mtl -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-d3d12 -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4);
RWStructuredBuffer<int> outputBuffer;

[NumThreads(1,1,1)]
void computeMain()
{
    static groupshared Atomic<int> atomicInt;
    atomicInt = 0;

    bool result = true;
    if (atomicInt.add(1) != 0)
        result = false;
    if (atomicInt.sub(1) != 1)
        result = false;
    if (atomicInt.max(2) != 0)
        result = false;
    if (atomicInt.min(1) != 2)
        result = false;
    if (atomicInt.or(3) != 1)
        result = false;
    if (atomicInt.and(2) != 3)
        result = false;
    if (atomicInt.xor(3) != 2)
        result = false;
    if (atomicInt.exchange(4) != 1)
        result = false;
    if (atomicInt.compareExchange(4, 5) != 4)
        result = false;
    if (atomicInt.load() != 5)
        result = false;
    if (atomicInt.increment() != 5)
        result = false;
    if (atomicInt.decrement() != 6)
        result = false;
    atomicInt.store(6);
    if (atomicInt.load() != 6)
        result = false;
    // CHECK: 1
    if (result)
        outputBuffer[0] = 1;
    else
        outputBuffer[0] = 0;
}