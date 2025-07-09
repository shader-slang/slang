// TEST:SIMPLE(filecheck=CHECK):-stage compute -entry computeMain -target spirv

RWStructuredBuffer<int> processMemory;
RWStructuredBuffer<int> output;

__generic<AddressSpace a, CoherentScope c>
void writeToReadOnlyPointer(Ptr<int, a, PtrAccess::Read, c> ptr)
{
    // CHECK: error 41404
    ptr[0] = 1;
}

[numthreads(1, 1, 1)]
void computeMain(int id : SV_DispatchThreadID)
{
    Ptr<int, AddressSpace::UserPointer, PtrAccess::Read, CoherentScope::Device> ptr = &processMemory[id.x];

    // CHECK: error 41404
    ptr[id+1] = 1;

    // CHECK: error 41404
    *ptr = 2;

    writeToReadOnlyPointer(ptr);
    output[id] = ptr[id];
}
