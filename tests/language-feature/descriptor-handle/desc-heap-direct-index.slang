// Regression/feature test for shader-slang/slang#11568:
// Allow HLSL SM6.6 ResourceDescriptorHeap[i] / SamplerDescriptorHeap[i] directly in Slang source.
// The resource type is recovered from the assignment target via implicit conversion from the
// untyped heap handle, then rides the existing DescriptorHandle<T> lowering: HLSL emits the SM6.6
// heap-index strings; SPIR-V uses SPV_EXT_descriptor_heap.

//TEST:SIMPLE(filecheck=HLSL): -target hlsl -stage compute -entry computeMain -profile cs_6_6
//TEST:SIMPLE(filecheck=SPV): -target spirv-asm -stage compute -entry computeMain -capability spvDescriptorHeapEXT

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // T solved from the declared variable type, not from the index argument.
    Texture2D t = ResourceDescriptorHeap[tid.x];
    SamplerState s = SamplerDescriptorHeap[tid.y];
    // Also exercise the .Handle (DescriptorHandle<T>) conversion spelling.
    Texture2D.Handle th = ResourceDescriptorHeap[tid.x];
    Texture2D t2 = th;
    outputBuffer[0] = t.SampleLevel(s, float2(0.0, 0.0), 0.0).x + t2.SampleLevel(s, float2(0.0, 0.0), 0.0).x;
}

// HLSL: ResourceDescriptorHeap[
// HLSL: SamplerDescriptorHeap[

// SPV: OpExtension "SPV_EXT_descriptor_heap"
