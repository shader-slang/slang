// The stack frame allocator is only available on the CPU and CUDA targets.
// The LLVM results are checked separately, as it is the only target here that
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cpu -compute
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -llvm -compute

//TEST_INPUT:set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

bool isAligned<T>(Ptr<T> aligned, size_t expectedAlignment)
{
    return (reinterpret<uintptr_t>(aligned) & (expectedAlignment-1)) == 0;
}

void alignTest()
{
    // The order is intentionally small->high to ensure that naively allocating
    // the next free pointer would cause misalignment.
    // CHECK: 1
    outputBuffer[0] = isAligned(StackFrameAllocator.allocate<uint8_t, DefaultDataLayout>(), 1) ? 1 : 0;
    // CHECK-NEXT: 1
    outputBuffer[1] = isAligned(StackFrameAllocator.allocate<uint16_t, DefaultDataLayout>(), 2) ? 1 : 0;
    // CHECK-NEXT: 1
    outputBuffer[2] = isAligned(StackFrameAllocator.allocate<uint, DefaultDataLayout>(), 4) ? 1 : 0;
    // CHECK-NEXT: 1
    outputBuffer[3] = isAligned(StackFrameAllocator.allocate<uint64_t, DefaultDataLayout>(), 8) ? 1 : 0;
    // CHECK-NEXT: 1
    outputBuffer[4] = isAligned(StackFrameAllocator.allocate<float4, DefaultDataLayout>(), 16) ? 1 : 0;
}

bool loopTest()
{
    // We should get a different pointer on each iteration; hoisting the alloca
    // is not correct behavior here.
    int* ints = StackFrameAllocator.allocate<int, DefaultDataLayout>(8);
    for (int i = 0; i < 4; ++i)
    {
        int* localInts = StackFrameAllocator.allocate<int, DefaultDataLayout>(8);
        if (localInts == ints)
            return false;
        ints = localInts;
    }
    return true;
}

bool reallocTest<T: __BuiltinIntegerType>()
{
    T* ptr = StackFrameAllocator.allocate<T, DefaultDataLayout>(8);
    for (int i = 0; i < 8; ++i)
    {
        ptr[i] = T(10+i);
    }

    T* rePtr;
    do
    {
        rePtr = StackFrameAllocator.reallocate<T, DefaultDataLayout>(ptr, 8, 16);
    }
    catch
    {
        return false;
    }
    for (int i = 0; i < 8; ++i)
        if (rePtr[i] != T(10+i))
            return false;

    do
    {
        rePtr = StackFrameAllocator.reallocate<T, DefaultDataLayout>(ptr, 16, 8);
    }
    catch
    {
        return false;
    }
    for (int i = 0; i < 8; ++i)
        if (rePtr[i] != T(10+i))
            return false;
    return true;
}

[numthreads(1,1,1)]
void computeMain()
{
    alignTest();
    // CHECK-NEXT: 1
    outputBuffer[5] = loopTest() ? 1 : 0;

    // CHECK-NEXT: 1
    outputBuffer[6] = reallocTest<uint8_t>() ? 1 : 0;
    outputBuffer[7] = reallocTest<uint16_t>() ? 1 : 0;
    outputBuffer[8] = reallocTest<uint>() ? 1 : 0;
    outputBuffer[9] = reallocTest<uint64_t>() ? 1 : 0;

    // CHECK-NEXT: 1
    do
    {
        // Dynamically-aligned allocations aren't supported on the stack.
        void* ptr = try StackFrameAllocator.alignedAllocate(256, 4);
        outputBuffer[10] = 0;
    }
    catch (err: MemoryAllocationError)
    {
        outputBuffer[10] = err == MemoryAllocationError.AlignmentIsTooLarge ? 1 : 2;
    }

    // Unfortunately, there is no way to test the data layout functionality - it
    // only changes the size of the allocation in bytes, which the allocators do
    // not actually communicate.
}
