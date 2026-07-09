// Regression test for #11681: an implicit conversion from
// `DescriptorHandle<ConstantBuffer<T>>` to `ConstantBuffer<T>` must succeed,
// matching the documented `DescriptorHandle<T> -> T` behavior for any
// `T : IOpaqueDescriptor` (`ConstantBuffer` is a parameter-group type that
// conforms). Before the fix, the blanket "disallow converting to a
// ParameterGroupType" guard in `_coerce()` rejected this assignment with
// `error[E30019]: type mismatch`, even though the compiler-generated
// `__init(DescriptorHandle<This>)` implicit conversion exists and lowers
// correctly. A `RWStructuredBuffer<uint>` source (not a parameter-group type)
// already worked; this locks in matching behavior for `ConstantBuffer`.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry main -stage compute -capability spvDescriptorHeapEXT

struct Uniforms
{
    uint x;
}

[shader("compute")]
[NumThreads(1, 1, 1)]
void main(
    uniform DescriptorHandle<ConstantBuffer<Uniforms>> input,
    uniform DescriptorHandle<RWStructuredBuffer<uint>> output)
{
    // Implicit conversions, with no explicit `getDescriptorFromHandle()` call:
    ConstantBuffer<Uniforms> cb = input;
    RWStructuredBuffer<uint> outBuf = output;
    outBuf[0] = cb.x;
}

// A successful compile (no E30019) emits an entry point and lowers the
// ConstantBuffer descriptor-heap access to a buffer pointer in the StorageBuffer
// storage class -- identical to the explicit `getDescriptorFromHandle()` path
// validated by tests/spirv/descriptor-heap-constant-buffer.slang. (Since #11483
// the descriptor-heap constant buffer is lowered to a StorageBuffer-class buffer
// pointer rather than Uniform, so its nested-array pointer types get strides.)
// CHECK: OpEntryPoint
// CHECK: OpTypePointer StorageBuffer %Uniforms_std140
// CHECK: OpBufferPointerEXT %_ptr_StorageBuffer_Uniforms_std140
