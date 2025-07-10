//TEST:SIMPLE(filecheck=CHECK):-stage compute -entry computeMain -target spirv

// Tests valid use of read-only pointer

// CHECK: OpEntryPoint
// CHECK-NOT: error

RWStructuredBuffer<int> processMemory;
RWStructuredBuffer<int> output;

__generic<AddressSpace a, CoherentScope c>
void writeToReadOnlyPointer( Ptr<int, a, Access::Read, c> ptr)
{
    ptr[0] = 1;
}

[numthreads(1, 1, 1)]
void computeMain(int id : SV_DispatchThreadID)
{
    Ptr<int, AddressSpace::UserPointer, Access::Read> ptr1 = &processMemory[id.x];
    CoherentPtr<int, Access::Read> ptr2 = CoherentPtr<int, Access::Read>(&processMemory[id.x]);

    output[id] = ptr1[id];
    output[id+32] = ptr2[id];
}
