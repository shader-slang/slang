// The system heap allocator is only available on the CPU and CUDA targets.
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cpu -compute
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -llvm -compute

// Unfortunately, there is no way to directly test the data layout
// functionality - it only changes the size of the allocation in bytes, which
// the allocators do not actually communicate.

//TEST_INPUT:set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

bool isAligned<T>(Ptr<T> aligned, size_t expectedAlignment)
{
    return (reinterpret<uintptr_t>(aligned) & (expectedAlignment-1)) == 0;
}

void staticAlignTest() throws MemoryAllocationError
{
    bool cases[5] = {true,true,true,true,true};

    for (int i = 0; i < 20; ++i)
    {
        // The order is intentionally small->high to ensure that naively allocating
        // the next free pointer would cause misalignment. Frees are interlaced
        // to cause additional complications.

        uint8_t* u8ptr = try SystemHeapAllocator.allocate<uint8_t, DefaultDataLayout>();
        cases[0] = cases[0] && isAligned(u8ptr, 1);

        uint16_t* u16ptr = try SystemHeapAllocator.allocate<uint16_t, DefaultDataLayout>();
        cases[1] = cases[1] && isAligned(u16ptr, 2);
        SystemHeapAllocator.free(u8ptr);

        uint* uptr = try SystemHeapAllocator.allocate<uint, DefaultDataLayout>();
        cases[2] = cases[2] && isAligned(uptr, 4);
        SystemHeapAllocator.free(u16ptr);

        uint64_t* u64ptr = try SystemHeapAllocator.allocate<uint64_t, DefaultDataLayout>();
        cases[3] = cases[3] && isAligned(u64ptr, 8);
        SystemHeapAllocator.free(uptr);

        float4* f4ptr = try SystemHeapAllocator.allocate<float4, DefaultDataLayout>();
        cases[4] = cases[4] && isAligned(f4ptr, 16);
        SystemHeapAllocator.free(u64ptr);
        SystemHeapAllocator.free(f4ptr);
    }

    // CHECK: 1
    outputBuffer[0] = cases[0] ? 1 : 0;
    // CHECK-NEXT: 1
    outputBuffer[1] = cases[1] ? 1 : 0;
    // CHECK-NEXT: 1
    outputBuffer[2] = cases[2] ? 1 : 0;
    // CHECK-NEXT: 1
    outputBuffer[3] = cases[3] ? 1 : 0;
    // CHECK-NEXT: 1
    outputBuffer[4] = cases[4] ? 1 : 0;
}

int dynamicAlignTest()
{
    // Repeatedly attempt dynamic allocations with various alignments
    for (int j = 0; j < 20; ++j)
    {
        for (int i = 0; i <= 12; ++i)
        {
            int alignment = 1<<i;
            void* addr = nullptr;
            do
            {
                addr = try SystemHeapAllocator.alignedAllocate(alignment*2, alignment);
            }
            catch
            {
                return alignment;
            }
            if (!addr || !isAligned(addr, alignment))
                return alignment;
            SystemHeapAllocator.alignedFree(addr);
        }
    }
    return 0;
}

struct CustomStruct
{
    float3 a;
    float b;
};

bool reallocTest() throws MemoryAllocationError
{
    // Write data, realloc a bit and check that it's still all there.
    for (int j = 0; j < 20; ++j)
    {
        int l1 = 2 * j;
        int l2 = 4 * j;
        CustomStruct* addr = try SystemHeapAllocator.allocate<CustomStruct, DefaultDataLayout>(l1);
        for (int i = 0; i < l1; ++i)
        {
            addr[i].a = float3(1.0,2.0,3.0) + i * 4;
            addr[i].b = 4.0 + i;
        }

        addr = try SystemHeapAllocator.reallocate<CustomStruct, DefaultDataLayout>(addr, l1, l2);
        for (int i = 0; i < l1; ++i)
            if (any(addr[i].a != float3(1.0,2.0,3.0) + i * 4) || addr[i].b != 4.0 + i)
                return false;

        addr = try SystemHeapAllocator.reallocate<CustomStruct, DefaultDataLayout>(addr, l2, l1);
        for (int i = 0; i < l1; ++i)
            if (any(addr[i].a != float3(1.0,2.0,3.0) + i * 4) || addr[i].b != 4.0 + i)
                return false;

        SystemHeapAllocator.free(addr);
    }
    return true;
}

[numthreads(1,1,1)]
void computeMain()
{
    do
    {
        try staticAlignTest();
    }
    catch
    {
        // Failure
        outputBuffer[0] = 0;
        outputBuffer[1] = 0;
        outputBuffer[2] = 0;
        outputBuffer[3] = 0;
        outputBuffer[4] = 0;
    }

    // CHECK-NEXT: 0
    outputBuffer[5] = dynamicAlignTest();

    // CHECK-NEXT: 1
    do
    {
        outputBuffer[6] = try reallocTest() ? 1 : 0;
    }
    catch
    {
        // Failure
        outputBuffer[6] = 0;
    }
}
