//TEST:SIMPLE(filecheck=CHECK):-stage compute -entry computeMain -target spirv

// Tests pointer casting rules: Only explicit casting is allowed between pointer types.
// All implicit conversions between pointer types should fail.
int* processMemory;
RWStructuredBuffer<int> output;

[numthreads(1, 1, 1)]
void computeMain(int id : SV_DispatchThreadID)
{
    // regular address-of
    // CHECK-NOT: ([[# @LINE+1]]): error
    Ptr<int, Access::ReadWrite, AddressSpace::Device> rwPtr = processMemory + id.x;
    // copying a pointer of T* syntax
    // CHECK-NOT: ([[# @LINE+1]]): error
    Ptr<int, Access::ReadWrite, AddressSpace::Device> copiedPtrOfLegacySyntax = processMemory;
    // casting to Read ptr
    // CHECK-NOT: ([[# @LINE+1]]): error
    Ptr<int, Access::Read> rPtr = Ptr<int, Access::Read>(processMemory + id.x);

    // casting to RW ptr from a R ptr
    // CHECK-NOT: ([[# @LINE+1]]): error
    Ptr<int, Access::ReadWrite, AddressSpace::Device> p1 = Ptr<int, Access::ReadWrite, AddressSpace::Device>(rPtr);
    // casting to R ptr from a RW ptr
    // CHECK-NOT: ([[# @LINE+1]]): error
    Ptr<int, Access::Read, AddressSpace::Device> p2 = Ptr<int, Access::Read, AddressSpace::Device>(rwPtr);
    // casting to ptr of different type
    // CHECK-NOT: ([[# @LINE+1]]): error
    Ptr<float, Access::ReadWrite, AddressSpace::Device> p3 = Ptr<float, Access::ReadWrite, AddressSpace::Device>(rPtr);

    // Cannot implicit cast ptr's
    // CHECK: ([[# @LINE+1]]): error 30019
    Ptr<float, Access::ReadWrite, AddressSpace::Device> p4 = rPtr;
    // cannot implcitly cast between different access qualifiers
    // CHECK: ([[# @LINE+1]]): error 30019
    Ptr<int, Access::Read> p5 = Ptr<int, Access::ReadWrite>(processMemory + id.x);
    // cannot implcitly cast between different access qualifiers
    // CHECK: ([[# @LINE+1]]): error 30019
    Ptr<int, Access::ReadWrite> p6 = Ptr<int, Access::Read>(processMemory + id.x);

    // TODO: Enable this when we allow user-defined group-shared address space, Issue #8173.
    // Cannot cast between different address spaces.
    // CHECK: ([[# @LINE+1]]): error
    Ptr<float, Access::ReadWrite, AddressSpace::GroupShared> p7 = Ptr<float, Access::ReadWrite, AddressSpace::GroupShared>(rwPtr);
    // CHECK: ([[# @LINE+1]]): error
    Ptr<float, Access::ReadWrite, AddressSpace::GroupShared> p8 = Ptr<float, Access::ReadWrite, AddressSpace::GroupShared>(p1);
    // CHECK: ([[# @LINE+1]]): error
    Ptr<float, Access::ReadWrite, AddressSpace::GroupShared> p9 = rwPtr;

    output[id] = *rwPtr;
}
