// Pointer fields in interface implementations produce invalid SPIRV when
// packed into AnyValue for dynamic dispatch. SPIRV generates incompatible
// struct types for Function vs PhysicalStorageBuffer storage classes,
// causing type mismatches in the generated code.
//
// CPU and CUDA handle pointer-in-AnyValue correctly (pointers are just
// raw addresses), so this diagnostic is SPIRV-specific. Interface pointers
// (IFoo*) are allowed since they point to existential tuples handled by
// dynamic dispatch lowering.
//
// See #10398 and #10399.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -conformance "PtrImpl:IFoo=0" -conformance "MultiPtrImpl:IFoo=1" -conformance "NestedPtrImpl:IFoo=2" -conformance "ArrayPtrImpl:IFoo=3" -conformance "GoodImpl:IFoo=4" -conformance "GoodInterfacePtrImpl:IFoo=5"

interface IFoo
{
    int get();
}

// Direct pointer field - rejected on SPIRV
export struct PtrImpl : IFoo
//CHECK:      ^^^^^^^ type cannot be packed for dynamic dispatch
//CHECK:      ^^^^^^^ type 'PtrImpl' contains fields that cannot be packed into ordinary bytes for dynamic dispatch.
{
    int* p;
    int get() { return *p; }
}

// Multiple pointer fields - rejected on SPIRV
export struct MultiPtrImpl : IFoo
//CHECK:      ^^^^^^^^^^^^ type cannot be packed for dynamic dispatch
//CHECK:      ^^^^^^^^^^^^ type 'MultiPtrImpl' contains fields that cannot be packed into ordinary bytes for dynamic dispatch.
{
    int* a;
    float* b;
    int get() { return *a + int(*b); }
}

// Nested pointer via struct field - tests recursive checking
struct Inner
{
    int* p;
}

export struct NestedPtrImpl : IFoo
//CHECK:      ^^^^^^^^^^^^^ type cannot be packed for dynamic dispatch
//CHECK:      ^^^^^^^^^^^^^ type 'NestedPtrImpl' contains fields that cannot be packed into ordinary bytes for dynamic dispatch.
{
    Inner inner;
    int get() { return *inner.p; }
}

// Array of pointers - tests ArrayType -> PtrType recursion
export struct ArrayPtrImpl : IFoo
//CHECK:      ^^^^^^^^^^^^ type cannot be packed for dynamic dispatch
//CHECK:      ^^^^^^^^^^^^ type 'ArrayPtrImpl' contains fields that cannot be packed into ordinary bytes for dynamic dispatch.
{
    int* arr[2];
    int get() { return *arr[0]; }
}

// Good: no pointer fields
export struct GoodImpl : IFoo
{
    int v;
    int get() { return v; }
}

// Good: interface pointer is allowed (existential tuple, not concrete struct)
export struct GoodInterfacePtrImpl : IFoo
{
    IFoo* next;
    int get() { return 1; }
}

uniform int dynId;
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    IFoo f = createDynamicObject<IFoo>(dynId, 0);
    outputBuffer[0] = f.get();
}
