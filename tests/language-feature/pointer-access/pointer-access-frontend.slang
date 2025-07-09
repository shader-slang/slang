// TEST:SIMPLE(filecheck=CHECK):-stage compute -entry computeMain -target spirv

RWStructuredBuffer<int> processMemory;
RWStructuredBuffer<int> output;

#define SUBGROUP_SIZE 8
#define HALF_SUBGROUP_SIZE SUBGROUP_SIZE/2
[numthreads(SUBGROUP_SIZE, 1, 1)]
void computeMain(int id : SV_DispatchThreadID)
{
    Ptr<int, AddressSpace::UserPointer, PtrAccess::Read, CoherentScope::Device> ptr = &processMemory[id.x];
    Ptr<int, AddressSpace::UserPointer, PtrAccess::ReadWrite, CoherentScope::Subgroup> ptr2 = &processMemory[id.x + 5];
    CoherentPtr<int, AddressSpace::UserPointer> ptr2 = &processMemory[id.x + 4];
}
