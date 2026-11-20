// Regression test for https://github.com/shader-slang/slang/issues/14652.
//
// A ByteAddressBuffer parameter whose argument comes from the descriptor heap cannot be
// specialized away before byte-address-buffer legalization, so its load/store operations
// reach the pass with the parameter still in place. The pass used to retype the shared
// ByteAddressBuffer type instruction module-wide when legalizing such a parameter, so a
// global ByteAddressBuffer loaded at a different element type was silently retyped too and
// its load reused the first-legalized element type, producing invalid SPIR-V that fails
// spirv-val with a type-mismatched OpAccessChain.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry main -stage compute -capability spvDescriptorHeapEXT -validate-with-downstream-compiler
//TEST:SIMPLE(filecheck=SWAP): -target spirv-asm -entry mainSwapped -stage compute -capability spvDescriptorHeapEXT -validate-with-downstream-compiler

ByteAddressBuffer buf;
RWStructuredBuffer<float> outBuf;

uint readIndex(ByteAddressBuffer b, uint offset)
{
    return b.Load(offset);
}

float2 readFloat2(ByteAddressBuffer b, uint offset)
{
    return b.Load<float2>(offset);
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

// CHECK: OpCapability DescriptorHeapEXT

// SWAP: OpCapability DescriptorHeapEXT
