//TEST:SIMPLE(filecheck=CHECK):-stage compute -entry computeMain -target spirv

// Writing with a read-only pointer should be an error

RWStructuredBuffer<int> processMemory;
RWStructuredBuffer<int> output;

typealias ReadPtr = Ptr<int, Access::Read, AddressSpace::UserPointer>;

void writeToReadOnlyPointer(ReadPtr ptr)
{
    // CHECK: ([[# @LINE+1]]): error 41404
    ptr[0] = 1;
}

[numthreads(1, 1, 1)]
void computeMain(int id : SV_DispatchThreadID)
{
    ReadPtr ptr1 = ReadPtr(&processMemory[id.x]);

    // CHECK: ([[# @LINE+1]]): error 41404
    ptr1[id + 1] = 1;
    // CHECK: ([[# @LINE+1]]): error 41404
    *ptr1 = 1;
    writeToReadOnlyPointer(ptr1);
    output[id] = ptr1[id];
}
