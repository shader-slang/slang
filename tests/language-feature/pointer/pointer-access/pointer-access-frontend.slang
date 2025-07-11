// TEST:SIMPLE(filecheck=CHECK):-stage compute -entry computeMain -target spirv

//CHECK: OpEntryPoint
//CHECK-NOT: error

RWStructuredBuffer<int> processMemory;
RWStructuredBuffer<int> output;

[numthreads(1, 1, 1)]
void computeMain(int id : SV_DispatchThreadID)
{
    Ptr<int, AddressSpace::UserPointer, Access::ReadWrite, CoherentScope::NotCoherent> ptr1 = &processMemory[id.x + 5];
    Ptr<int> ptr2 = &processMemory[id.x + 4];
    CoherentPtr<int> ptr3 = CoherentPtr<int>(&processMemory[id.x + 4]);
}
