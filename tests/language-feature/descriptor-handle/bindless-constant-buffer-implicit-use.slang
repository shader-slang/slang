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
// ConstantBuffer descriptor-heap access to a uniform-buffer descriptor with an untyped
// Uniform data pointer whose members are addressed through `OpUntypedAccessChainKHR` --
// identical to the explicit `getDescriptorFromHandle()` path validated by
// tests/spirv/descriptor-heap-constant-buffer.slang.
// CHECK: OpEntryPoint
// CHECK: OpTypeBufferEXT Uniform
// CHECK: %[[UNIFORM_PTR:_ptr_Uniform]] = OpTypeUntypedPointerKHR Uniform{{$}}
// CHECK: OpBufferPointerEXT %[[UNIFORM_PTR]]
// CHECK: OpUntypedAccessChainKHR %[[UNIFORM_PTR]] %Uniforms_std140
// CHECK-NOT: OpTypePointer StorageBuffer %Uniforms_std140
