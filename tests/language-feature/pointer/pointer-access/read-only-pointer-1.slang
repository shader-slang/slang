//TEST:SIMPLE(filecheck=CHECK):-stage compute -entry computeMain -target spirv

// Writing with a read-only pointer should be an error

RWStructuredBuffer<int> processMemory;
RWStructuredBuffer<int> output;

__generic<AddressSpace a, CoherentScope c>
void writeToReadOnlyPointer( Ptr<int, a, Access::Read, c> ptr)
{
    // CHECK: ([[# @LINE+1]]): error 41404
    ptr[0] = 1;
}

[numthreads(1, 1, 1)]
void computeMain(int id : SV_DispatchThreadID)
{
    Ptr<int, AddressSpace::UserPointer, Access::Read> ptr1 = &processMemory[id.x];
    CoherentPtr<int, Access::Read> ptr2 = CoherentPtr<int, Access::Read>(ptr1);

    // CHECK: ([[# @LINE+1]]): error 41404
    ptr1[id+1] = 1;

    // CHECK: ([[# @LINE+1]]): error 41404
    *ptr2 = 2;

    writeToReadOnlyPointer(ptr1);
    output[id] = ptr1[id];
}
