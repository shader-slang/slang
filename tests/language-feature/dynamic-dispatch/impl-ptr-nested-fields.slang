// Regression coverage for #10908: dyn interface impls whose pointer fields
// are reached via the recursive arms of containsUnmarshalableType
// (StructType -> PtrType, ArrayType -> PtrType) must also compile to valid
// SPIR-V via the AnyValue dynamic dispatch path. Mirrors the shapes the
// removed diagnose-ptr-in-impl-spirv.slang previously rejected.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -stage compute -entry main -conformance NestedPtrImpl:IFoo -conformance ArrayPtrImpl:IFoo

dyn interface IFoo
{
    int get();
};

struct Inner
{
    Ptr<int> p;
}

// Nested pointer reached via StructType recursion.
struct NestedPtrImpl : IFoo
{
    Inner inner;
    int get() { return *inner.p; }
}

// Array of pointers reached via ArrayType recursion.
struct ArrayPtrImpl : IFoo
{
    Ptr<int> arr[2];
    int get() { return *arr[0] + *arr[1]; }
}

RWStructuredBuffer<IFoo> input;
RWStructuredBuffer<int> output;

// CHECK: OpEntryPoint
// CHECK: OpBitcast %_ptr_PhysicalStorageBuffer_int
[shader("compute"), numthreads(1, 1, 1)]
void main(uint3 tid: SV_DispatchThreadID)
{
    output[0] = input[0].get();
}
