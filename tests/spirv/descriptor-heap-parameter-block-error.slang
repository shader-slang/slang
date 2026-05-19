// Verify that DescriptorHandle<ParameterBlock<T>> is rejected at the front end
// because ParameterBlock<T> does not conform to IOpaqueDescriptor.
// This ensures that if IOpaqueDescriptor conformance rules change, CI catches
// whether ParameterBlock would then need to be handled in the legalize pass.

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target spirv -entry main -stage compute -capability spvDescriptorHeapEXT

struct Uniforms { uint x; }

[shader("compute")]
[NumThreads(1, 1, 1)]
void main(
    // CHECK: error[E38029]
    uniform DescriptorHandle<ParameterBlock<Uniforms>> input,
    uniform DescriptorHandle<RWStructuredBuffer<uint>> output)
{
    output[0] = getDescriptorFromHandle(input).x;
}
