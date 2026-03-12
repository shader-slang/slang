//TEST:SIMPLE(filecheck=CHECK):-stage compute -entry computeMain -target spirv

// Writing with a read-only pointer should be an error

int* processMemory;
RWStructuredBuffer<int> output;

typealias ReadPtr = Ptr<int, Access::Read, AddressSpace::Device>;

void writeToReadOnlyPointer(ReadPtr ptr)
{
    // CHECK: error[E30011]
    ptr[0] = 1;
}

void writeToReadOnlyPointerOut(out int ptrVal)
{
    ptrVal = 1;
}

[numthreads(1, 1, 1)]
void computeMain(int id : SV_DispatchThreadID)
{
    // CHECK-NOT: error
    ReadPtr ptr1 = ReadPtr(processMemory + id.x);

    // CHECK: error[E30011]
    ptr1[id + 1] = 1;
    // CHECK: error[E30011]
    *ptr1 = 1;

    writeToReadOnlyPointer(ptr1);

    // CHECK: error[E30047]
    writeToReadOnlyPointerOut(ptr1[1]);

    // CHECK: error[E30047]
    writeToReadOnlyPointerOut(*(ptr1+2));
    
    output[id] = ptr1[id];
}
