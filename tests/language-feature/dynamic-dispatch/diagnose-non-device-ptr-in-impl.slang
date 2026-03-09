// Non-device address space pointers (groupshared, __ref) cannot appear as
// struct fields in Slang. This is a general language restriction (error 31201)
// that indirectly prevents non-device pointers from being used in dynamic
// dispatch implementations. Without this restriction, such pointers would
// reference thread-local or workgroup-local memory that becomes invalid
// after AnyValue round-tripping.
//
// Plain pointers (int*) default to device address space and are allowed
// in interface implementations.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -conformance "ImplGroupsharedPtr:IFoo=0" -conformance "ImplRefField:IFoo=1" -conformance "GoodImpl:IFoo=2" -conformance "GoodImplDevicePtr:IFoo=3"
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -stage compute -entry computeMain -conformance "ImplGroupsharedPtr:IFoo=0" -conformance "ImplRefField:IFoo=1" -conformance "GoodImpl:IFoo=2" -conformance "GoodImplDevicePtr:IFoo=3"

interface IFoo
{
    int get();
}

export struct ImplGroupsharedPtr : IFoo
{
    groupshared int* p;
/*CHECK:
    ^^^^^^^^^^^ modifier not allowed
    ^^^^^^^^^^^ modifier 'groupshared' is not allowed here.
*/
    int get() { return 1; }
}

export struct ImplRefField : IFoo
{
    __ref int r;
/*CHECK:
    ^^^^^ modifier not allowed
    ^^^^^ modifier '__ref' is not allowed here.
*/
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
    IFoo f = createDynamicObject<IFoo>(2, 0);
    outputBuffer[0] = f.get();
}
