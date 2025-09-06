//TEST:SIMPLE(filecheck=CHECK):-stage compute -entry computeMain -target spirv

//CHECK: OpEntryPoint
//CHECK-NOT: error

int* processMemory;
int* output;

[numthreads(1, 1, 1)]
void computeMain(int id : SV_DispatchThreadID)
{
    Ptr<int, Access::ReadWrite, AddressSpace::Device> ptr1 = processMemory + id.x + 5;
    Ptr<int> ptr2 = processMemory + id.x + 4;
}