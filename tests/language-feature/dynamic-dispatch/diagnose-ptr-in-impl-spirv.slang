// Pointer fields in interface implementations produce invalid SPIRV when
// packed into AnyValue for dynamic dispatch. SPIRV generates incompatible
// struct types for Function vs PhysicalStorageBuffer storage classes,
// causing type mismatches in the generated code.
//
// CPU and CUDA handle pointer-in-AnyValue correctly (pointers are just
// raw addresses), so this diagnostic is SPIRV-specific.
//
// See #10398 and #10399.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -conformance "PtrImpl:IFoo=0" -conformance "MultiPtrImpl:IFoo=1" -conformance "GoodImpl:IFoo=2"

interface IFoo
{
    int get();
}

export struct PtrImpl : IFoo
//CHECK:      ^^^^^^^ type cannot be packed for dynamic dispatch
//CHECK:      ^^^^^^^ type 'PtrImpl' contains fields that cannot be packed into ordinary bytes for dynamic dispatch.
{
    int* p;
    int get() { return *p; }
}

export struct MultiPtrImpl : IFoo
//CHECK:      ^^^^^^^^^^^^ type cannot be packed for dynamic dispatch
//CHECK:      ^^^^^^^^^^^^ type 'MultiPtrImpl' contains fields that cannot be packed into ordinary bytes for dynamic dispatch.
{
    int* a;
    float* b;
    int get() { return *a + int(*b); }
}

export struct GoodImpl : IFoo
{
    int v;
    int get() { return v; }
}

uniform int dynId;
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    IFoo f = createDynamicObject<IFoo>(dynId, 0);
    outputBuffer[0] = f.get();
}
