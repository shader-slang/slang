// Regression test for https://github.com/shader-slang/slang/issues/14652.
//
// Byte-address-buffer loads on SPIR-V legalize into loads from equivalent
// structured-buffer views, one per (buffer, element type). A ByteAddressBuffer
// parameter whose argument comes from the descriptor heap used to survive
// call specialization, so legalizing its load retyped the shared
// ByteAddressBuffer type instruction module-wide and a differently-typed load
// elsewhere reused the wrongly-typed view, failing spirv-val with a
// type-mismatched OpAccessChain. Call specialization now recognizes a
// descriptor-heap fetch as a specializable argument (it is a pure opaque-handle
// selection with the same (heap, index) shape as an element access), so the
// fetch moves into the specialized callee and every access takes its own
// element-type view.
//
// Every entry is gated on spirv-val via -validate-with-downstream-compiler,
// and the CHECK lines additionally pin the emitted typed structured-buffer
// blocks, so the test still fails on a broken compiler on machines where the
// harness strips the validator option: a view at the accessed element type
// (e.g. a float-element block for `buf`) never exists on the broken compiler,
// because that is the bug.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry main -stage compute -capability spvDescriptorHeapEXT -validate-with-downstream-compiler
//TEST:SIMPLE(filecheck=SWAP): -target spirv-asm -entry mainSwapped -stage compute -capability spvDescriptorHeapEXT -validate-with-downstream-compiler
//TEST:SIMPLE(filecheck=HELPERS): -target spirv-asm -entry mainHeapHelpers -stage compute -capability spvDescriptorHeapEXT -validate-with-downstream-compiler
//TEST:SIMPLE(filecheck=NOINLINE): -target spirv-asm -entry mainNoInline -stage compute -capability spvDescriptorHeapEXT -validate-with-downstream-compiler
//TEST:SIMPLE(filecheck=RW): -target spirv-asm -entry mainRW -stage compute -capability spvDescriptorHeapEXT -validate-with-downstream-compiler

ByteAddressBuffer buf;
RWByteAddressBuffer rwBuf;
RWStructuredBuffer<float> outBuf;

uint readIndex(ByteAddressBuffer b, uint offset)
{
    return b.Load(offset);
}

float2 readFloat2(ByteAddressBuffer b, uint offset)
{
    return b.Load<float2>(offset);
}

[noinline]
uint readWordNoInline(ByteAddressBuffer b, uint offset)
{
    return b.Load(offset);
}

[noinline]
void writeWord(RWByteAddressBuffer b, uint offset, uint value)
{
    b.Store(offset, value);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint tid : SV_DispatchThreadID)
{
    ByteAddressBuffer heapBuf = ResourceDescriptorHeap[tid];
    uint i = readIndex(heapBuf, tid * 4);
    float2 v = buf.Load<float2>(tid * 8);
    outBuf[0] = v.x + v.y + float(i);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void mainSwapped(uint tid : SV_DispatchThreadID)
{
    // Mirror of `main`: the heap-passed parameter loads the vector type while the global
    // buffer loads a scalar, so the parameter's legalized type must not leak into `buf`.
    ByteAddressBuffer heapBuf = ResourceDescriptorHeap[tid];
    float2 v = readFloat2(heapBuf, tid * 8);
    uint i = buf.Load(tid * 4);
    outBuf[0] = v.x + v.y + float(i);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void mainHeapHelpers(uint3 tid : SV_DispatchThreadID)
{
    // Two helpers, both fed directly from the descriptor heap, loading at
    // different element types. Neither ByteAddressBuffer parameter may survive
    // into byte-address-buffer legalization: each heap fetch specializes into
    // its callee and each load takes its own element-type view.
    uint i = readIndex(ResourceDescriptorHeap[tid.x], 0);
    float2 v = readFloat2(ResourceDescriptorHeap[tid.y], 8);
    outBuf[0] = v.x + v.y + float(i);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void mainNoInline(uint3 tid : SV_DispatchThreadID)
{
    // A [noinline] helper keeps its function shape; specialization must still
    // rewrite its signature so the heap fetch happens inside the callee.
    uint i = readWordNoInline(ResourceDescriptorHeap[tid.x], 0);
    outBuf[0] = float(i);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void mainRW(uint3 tid : SV_DispatchThreadID)
{
    // The store path and the writable kind: a heap-sourced RWByteAddressBuffer
    // stored through a [noinline] helper, next to a global RWByteAddressBuffer
    // storing at a different element type. Both must take their own
    // element-type views; the writable parameter must not survive into
    // legalization and retype `rwBuf`.
    writeWord(ResourceDescriptorHeap[tid.x], 0, tid.y);
    rwBuf.Store<float2>(tid.z * 8, float2(1.0, 2.0));
    outBuf[0] = float(tid.z);
}

// The uint-element block is the view used by the heap-fetch callee; the
// float-element block is `buf`'s view at the accessed element type, which the
// broken compiler never produces (it reuses the uint view instead).
// CHECK: OpCapability DescriptorHeapEXT
// CHECK: %StructuredBuffer = OpTypeStruct %_runtimearr_uint
// CHECK: %StructuredBuffer_0 = OpTypeStruct %_runtimearr_float
// CHECK: OpAccessChain %_ptr_StorageBuffer_float %buf
// CHECK-NOT: ByteAddressBuffer

// SWAP: OpCapability DescriptorHeapEXT
// SWAP: %StructuredBuffer = OpTypeStruct %_runtimearr_float
// SWAP: %StructuredBuffer_0 = OpTypeStruct %_runtimearr_uint
// SWAP: OpAccessChain %_ptr_StorageBuffer_uint %buf
// SWAP-NOT: ByteAddressBuffer

// HELPERS: OpCapability DescriptorHeapEXT
// HELPERS: %StructuredBuffer = OpTypeStruct %_runtimearr_uint
// HELPERS: %StructuredBuffer_0 = OpTypeStruct %_runtimearr_float
// HELPERS-NOT: ByteAddressBuffer

// NOINLINE: OpCapability DescriptorHeapEXT
// NOINLINE: %StructuredBuffer = OpTypeStruct %_runtimearr_uint
// NOINLINE-NOT: ByteAddressBuffer

// RW: OpCapability DescriptorHeapEXT
// RW: %RWStructuredBuffer = OpTypeStruct %_runtimearr_uint
// RW: %RWStructuredBuffer_0 = OpTypeStruct %_runtimearr_float
// RW: OpAccessChain %_ptr_StorageBuffer_float %rwBuf
// RW-NOT: ByteAddressBuffer
