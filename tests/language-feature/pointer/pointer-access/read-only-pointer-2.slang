//TEST:SIMPLE(filecheck=CHECK):-stage compute -entry computeMain -target spirv

// Tests valid use of read-only pointer

// CHECK: OpEntryPoint
// CHECK-NOT: error

int* processMemory;
int* output;

typealias ReadPtr = Ptr<int, Access::Read, AddressSpace::Device>;

[numthreads(1, 1, 1)]
void computeMain(int id : SV_DispatchThreadID)
{
    ReadPtr ptr1 = ReadPtr(processMemory + id.x);

    output[id] = ptr1[id];
}
