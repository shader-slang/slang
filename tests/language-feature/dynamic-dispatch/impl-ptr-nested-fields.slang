// Regression coverage for #10908: dyn interface impls whose pointer fields
// are reached via the recursive arms of containsUnmarshalableType
// (StructType -> PtrType, ArrayType -> PtrType) must also compile to valid
// SPIR-V via the AnyValue dynamic dispatch path. Mirrors the shapes the
// removed diagnose-ptr-in-impl-spirv.slang previously rejected.
//
// Each impl uses a distinct pointee type so its pack/unpack produces a
// uniquely shaped OpBitcast in the emitted SPIR-V. Both must be present —
// dropping either impl's marshalling silently is observable.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -stage compute -entry main -conformance NestedPtrImpl:IFoo -conformance ArrayPtrImpl:IFoo

dyn interface IFoo
{
    float get();
};

struct Inner
{
    Ptr<int> p;
}

// Nested pointer reached via StructType -> PtrType recursion. Pointee is int.
struct NestedPtrImpl : IFoo
{
    Inner inner;
    float get() { return float(*inner.p); }
}

// Array of pointers reached via ArrayType -> PtrType recursion. Pointee is
// float so its bitcast is distinguishable from the nested impl's.
struct ArrayPtrImpl : IFoo
{
    Ptr<float> arr[2];
    float get() { return *arr[0] + *arr[1]; }
}

RWStructuredBuffer<IFoo> input;
RWStructuredBuffer<float> output;

// CHECK: OpEntryPoint
// NestedPtrImpl: ptr-to-int unpacked via OpBitcast, then converted to float.
// CHECK-DAG: OpBitcast %_ptr_PhysicalStorageBuffer_int
// CHECK-DAG: OpConvertSToF
// ArrayPtrImpl: two ptr-to-float unpacks via OpBitcast, then summed.
// CHECK-DAG: OpBitcast %_ptr_PhysicalStorageBuffer_float
// CHECK-DAG: OpBitcast %_ptr_PhysicalStorageBuffer_float
// CHECK-DAG: OpFAdd
[shader("compute"), numthreads(1, 1, 1)]
void main(uint3 tid: SV_DispatchThreadID)
{
    output[0] = input[0].get();
}
