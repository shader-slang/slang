//TEST:SIMPLE(filecheck=CHECK):-stage compute -entry computeMain -target spirv

// Tests pointer casting rules: Only explicit casting is allowed between pointer types.
// All implicit conversions between pointer types should fail.
RWStructuredBuffer<int> processMemory;
RWStructuredBuffer<int> output;

[numthreads(1, 1, 1)]
void computeMain(int id : SV_DispatchThreadID)
{
    // regular address-of
    // CHECK-NOT: ([[# @LINE+1]]): error
    int* rwPtr = &processMemory[id.x];
    // casting to Read ptr
    // CHECK-NOT: ([[# @LINE+1]]): error
    Ptr<int, Access::Read> rPtr = Ptr<int, Access::Read>(&processMemory[id.x]);
    
    // Cannot implicit cast ptr's
    // CHECK: ([[# @LINE+1]]): error
    Ptr<float, Access::ReadWrite, AddressSpace::UserPointer> p4 = rPtr;
    // cannot implcitly cast between different access qualifiers
    // CHECK: ([[# @LINE+1]]): error
    Ptr<int, Access::Read> p5 = Ptr<int, Access::ReadWrite>(&processMemory[id.x]);
    // cannot implcitly cast between different access qualifiers
    // CHECK: ([[# @LINE+1]]): error
    Ptr<int, Access::ReadWrite> p6 = Ptr<int, Access::Read>(&processMemory[id.x]);

    // Cannot cast between different address spaces.
    // CHECK: ([[# @LINE+1]]): error
    Ptr<float, Access::ReadWrite, AddressSpace::GroupShared> p7 = Ptr<float, Access::ReadWrite, AddressSpace::GroupShared>(rwPtr);
    // CHECK: ([[# @LINE+1]]): error
    Ptr<float, Access::ReadWrite, AddressSpace::GroupShared> p8 = Ptr<float, Access::ReadWrite, AddressSpace::GroupShared>(p1);
    // CHECK: ([[# @LINE+1]]): error
    Ptr<float, Access::ReadWrite, AddressSpace::GroupShared> p9 = rwPtr;

    // CHECK-NOT: error
    // casting to RW ptr from a R ptr
    Ptr<int, Access::ReadWrite, AddressSpace::UserPointer> p1 = Ptr<int, Access::ReadWrite, AddressSpace::UserPointer>(rPtr);
    // casting to R ptr from a RW ptr
    Ptr<int, Access::Read, AddressSpace::UserPointer> p2 = Ptr<int, Access::Read, AddressSpace::UserPointer>(rwPtr);
    // casting to ptr of different type
    Ptr<float, Access::ReadWrite, AddressSpace::UserPointer> p3 = Ptr<float, Access::ReadWrite, AddressSpace::UserPointer>(rPtr);


    output[id] = *rwPtr;
}
