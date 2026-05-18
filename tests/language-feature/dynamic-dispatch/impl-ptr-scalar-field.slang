// Regression test for #10908: a dyn interface impl with a Ptr<float> field
// must compile to valid SPIR-V via the AnyValue dynamic dispatch path. The
// pointer is packed/unpacked as raw bits (uint2 <-> ptr), so no
// Function-class pointee struct is created — there is no storage-class type
// duplication and validation passes.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -stage compute -entry main -conformance Foo:IFoo -conformance Foo2:IFoo

dyn interface IFoo
{
    float getValue();
};

struct Foo : IFoo
{
    Ptr<float> value;
    float getValue() { return *value; }
}

struct Foo2 : IFoo
{
    float getValue() { return 2; }
}

RWStructuredBuffer<IFoo> input;
RWStructuredBuffer<float> output;

// CHECK: OpEntryPoint
// CHECK: OpBitcast %_ptr_PhysicalStorageBuffer_float
[shader("compute"), numthreads(16, 16, 1)]
void main(uint3 tid: SV_DispatchThreadID)
{
    output[0] = input[0].getValue();
}
