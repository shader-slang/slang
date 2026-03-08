// Non-device address space pointers (groupshared, __ref) cannot appear as
// struct fields in Slang. This is a general language restriction (error 31201)
// that indirectly prevents non-device pointers from being used in dynamic
// dispatch implementations. Without this restriction, such pointers would
// reference thread-local or workgroup-local memory that becomes invalid
// after AnyValue round-tripping.
//
// Plain pointers (int*) default to device address space and are allowed
// in interface implementations.

//TEST:SIMPLE(filecheck=CHECK): -target spirv
//TEST:SIMPLE(filecheck=CHECK): -target hlsl -stage compute -entry computeMain

interface IFoo
{
    int get();
}

// Positive cases must not be rejected (no 41014 for GoodImpl or GoodImplDevicePtr).
// CHECK-NOT: type 'GoodImpl' contains fields that cannot be packed
// CHECK-NOT: type 'GoodImplDevicePtr' contains fields that cannot be packed

// CHECK-DAG: modifier 'groupshared' is not allowed here
export struct ImplGroupsharedPtr : IFoo
{
    groupshared int* p;
    int get() { return 1; }
}

// CHECK-DAG: modifier '__ref' is not allowed here
export struct ImplRefField : IFoo
{
    __ref int r;
    int get() { return 1; }
}

export struct GoodImpl : IFoo
{
    int v;
    int get() { return v; }
}

export struct GoodImplDevicePtr : IFoo
{
    int* p;
    int get() { return *p; }
}

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    IFoo f = createDynamicObject<IFoo>(0, 0);
    outputBuffer[0] = f.get();
}
