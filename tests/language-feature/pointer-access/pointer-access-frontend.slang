// TEST:SIMPLE(filecheck=CHECK):-stage compute -entry computeMain -target spirv

//CHECK: OpEntryPoint
//CHECK-NOT: error

RWStructuredBuffer<int> processMemory;
RWStructuredBuffer<int> output;

[numthreads(1, 1, 1)]
void computeMain(int id : SV_DispatchThreadID)
{
    Ptr<int, AddressSpace::UserPointer, PtrAccess::Read, CoherentScope::Device> ptr = &processMemory[id.x];
    Ptr<int, AddressSpace::UserPointer, PtrAccess::ReadWrite, CoherentScope::Subgroup> ptr2 = &processMemory[id.x + 5];
    CoherentPtr<int, AddressSpace::UserPointer> ptr2 = &processMemory[id.x + 4];
}
