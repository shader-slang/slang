// Regression test for descriptor-heap sourced ByteAddressBuffer values.
// Byte-address-buffer legalization must rewrite heap loads to equivalent
// structured-buffer heap loads before target emission.

//TEST:COMPARE_COMPUTE(filecheck-buffer=RUNTIME): -vk -shaderobj -output-using-type -render-feature bindless -emit-spirv-directly -Xslang -DRUNTIME
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry computeMain -stage compute -capability spvDescriptorHeapEXT -emit-spirv-directly
//TEST:SIMPLE(filecheck=FRAG): -target spirv-asm -entry fragmentMain -stage fragment -capability spvDescriptorHeapEXT -emit-spirv-directly
//TEST:SIMPLE(filecheck=DXIL): -target dxil -entry hlslMain -profile cs_6_6

// RUNTIME: type: uint
// RUNTIME-NEXT: 4
// RUNTIME-NEXT: 8
// RUNTIME-NEXT: 0
// RUNTIME-NEXT: 5

// CHECK: OpCapability DescriptorHeapEXT
// CHECK-NOT: ByteAddressBuffer
// CHECK: %v4uint = OpTypeVector %uint 4
// CHECK: %_runtimearr_v4uint = OpTypeRuntimeArray %v4uint
// CHECK: %StructuredBuffer = OpTypeStruct %_runtimearr_v4uint
// CHECK: %_ptr_StorageBuffer_StructuredBuffer = OpTypePointer StorageBuffer %StructuredBuffer
// CHECK: %_ptr_StorageBuffer_v4uint = OpTypePointer StorageBuffer %v4uint
// CHECK: %RWStructuredBuffer = OpTypeStruct %_runtimearr_uint
// CHECK: %_ptr_StorageBuffer_RWStructuredBuffer = OpTypePointer StorageBuffer %RWStructuredBuffer
// CHECK: OpBufferPointerEXT %_ptr_StorageBuffer_StructuredBuffer
// CHECK: OpAccessChain %_ptr_StorageBuffer_v4uint
// CHECK: OpLoad %v4uint
// CHECK: OpBufferPointerEXT %_ptr_StorageBuffer_RWStructuredBuffer
// CHECK: OpAccessChain %_ptr_StorageBuffer_uint
// CHECK: OpAtomicIAdd

// FRAG: OpCapability DescriptorHeapEXT
// FRAG-NOT: ByteAddressBuffer
// FRAG: %RasterizerOrderedStructuredBuffer = OpTypeStruct %_runtimearr_uint
// FRAG: %_ptr_StorageBuffer_RasterizerOrderedStructuredBuffer = OpTypePointer StorageBuffer %RasterizerOrderedStructuredBuffer
// FRAG: OpBufferPointerEXT %_ptr_StorageBuffer_RasterizerOrderedStructuredBuffer
// FRAG: OpAccessChain %_ptr_StorageBuffer_uint

// DXIL: result code = 0

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=output
RWStructuredBuffer<uint> output;

//TEST_INPUT: set readOnlyBuffer = ubuffer(data=[4 5 6 7], stride=4)
uniform ByteAddressBuffer.Handle readOnlyBuffer;

//TEST_INPUT: set readWriteBuffer = ubuffer(data=[0 0 0 0], stride=4)
uniform RWByteAddressBuffer.Handle readWriteBuffer;

//TEST_INPUT: set rasterOrderedBuffer = ubuffer(data=[0 0 0], stride=4)
uniform RasterizerOrderedByteAddressBuffer.Handle rasterOrderedBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    let vectorValue = readOnlyBuffer.Load<uint4>(0);
    let value0 = vectorValue.x;
    readWriteBuffer.Store<uint>(4, value0 * 2);

    uint original = 0;
    readWriteBuffer.InterlockedAdd(0, vectorValue.y, original);

    output[0] = value0;
    output[1] = readWriteBuffer.Load<uint>(4);
    output[2] = original;
    output[3] = readWriteBuffer.Load<uint>(0);
}

[numthreads(1, 1, 1)]
void hlslMain()
{
    // Public HLSL ByteAddressBuffer APIs emit native methods, so this entry
    // directly exercises equivalent-structured-buffer descriptor-heap lowering.
    let readOnlyStructured = __getEquivalentStructuredBuffer<uint>(readOnlyBuffer);
    let readWriteStructured = __getEquivalentStructuredBuffer<uint>(readWriteBuffer);

    output[0] = readOnlyStructured[0];
    output[1] = readWriteStructured[0];
    readWriteStructured[1] = 7;

    uint original = 0;
    InterlockedAdd(readWriteStructured[2], 5, original);
    output[2] = original;
}

[shader("fragment")]
float4 fragmentMain() : SV_Target
{
    rasterOrderedBuffer.Store<uint>(8, 1);
    return float4(rasterOrderedBuffer.Load<uint>(8), 0, 0, 1);
}
