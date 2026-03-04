//TEST:SIMPLE(filecheck=CHECK):-stage compute -entry computeMain -target spirv

// Tests pointer casting rules: Only explicit casting is allowed between pointer types.
// All implicit conversions between pointer types should fail.
int* processMemory;
RWStructuredBuffer<int> output;

[numthreads(1, 1, 1)]
void computeMain(int id : SV_DispatchThreadID)
{
    // regular address-of - no error expected
    Ptr<int, Access::ReadWrite, AddressSpace::Device> rwPtr = processMemory + id.x;
    // copying a pointer of T* syntax - no error expected
    Ptr<int, Access::ReadWrite, AddressSpace::Device> copiedPtrOfLegacySyntax = processMemory;
    // casting to Read ptr - no error expected
    Ptr<int, Access::Read> rPtr = Ptr<int, Access::Read>(processMemory + id.x);

    // casting to RW ptr from a R ptr - no error expected
    Ptr<int, Access::ReadWrite, AddressSpace::Device> p1 = Ptr<int, Access::ReadWrite, AddressSpace::Device>(rPtr);
    // casting to R ptr from a RW ptr - no error expected
    Ptr<int, Access::Read, AddressSpace::Device> p2 = Ptr<int, Access::Read, AddressSpace::Device>(rwPtr);
    // casting to ptr of different type - no error expected
    Ptr<float, Access::ReadWrite, AddressSpace::Device> p3 = Ptr<float, Access::ReadWrite, AddressSpace::Device>(rPtr);

    // Cannot implicit cast ptr's
    // CHECK: error[E30019]:
    Ptr<float, Access::ReadWrite, AddressSpace::Device> p4 = rPtr;
    // cannot implcitly cast between different access qualifiers
    // CHECK: error[E30019]:
    Ptr<int, Access::Read> p5 = Ptr<int, Access::ReadWrite>(processMemory + id.x);
    // cannot implcitly cast between different access qualifiers
    // CHECK: error[E30019]:
    Ptr<int, Access::ReadWrite> p6 = Ptr<int, Access::Read>(processMemory + id.x);

    // TODO: Enable this when we allow user-defined group-shared address space, Issue #8173.
    // Cannot cast between different address spaces.
    // CHECK: error[E30080]: ambiguous conversion
    Ptr<float, Access::ReadWrite, AddressSpace::GroupShared> p7 = Ptr<float, Access::ReadWrite, AddressSpace::GroupShared>(rwPtr);
    // CHECK: error[E30500]: too many initializers
    Ptr<float, Access::ReadWrite, AddressSpace::GroupShared> p8 = Ptr<float, Access::ReadWrite, AddressSpace::GroupShared>(p1);
    // CHECK: error[E30019]:
    Ptr<float, Access::ReadWrite, AddressSpace::GroupShared> p9 = rwPtr;

    output[id] = *rwPtr;
}
