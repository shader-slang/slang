//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -emit-spirv-directly -entry main -stage compute

// An element address with void pointee type is an opaque pointer computation, not storage for
// a None output. Void cleanup must retain it even though both use Ptr<void> in the IR.
// CHECK: OpAccessChain %_ptr_PhysicalStorageBuffer_uint

cbuffer Params
{
    void* address;
    uint index;
}

RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void main()
{
    void* element = __getElementPtr(address, index);
    outputBuffer[0] = reinterpret<uint64_t>(element) != 0;
}
