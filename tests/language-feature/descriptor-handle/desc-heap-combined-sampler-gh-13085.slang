// shader-slang/slang#13085: on targets whose combined-sampler lowering consumes a separate sampler
// index (HLSL and SPIR-V with `spvDescriptorHeapEXT`), a combined texture-sampler cannot be built
// from a single-index `ResourceDescriptorHeap[i]` handle -- doing so would silently default the
// sampler to heap slot 0. Both single-index conversion paths must be rejected (E41400) under those
// targets. Default SPIR-V and GLSL read a single native combined descriptor and are exercised as a
// positive case in desc-heap-combined-sampler-default-model-gh-13085.slang.

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target spirv -stage fragment -entry main -profile spirv_1_6 -capability spvDescriptorHeapEXT -emit-spirv-directly

[shader("fragment")]
float4 main(float3 coordinate : TEXCOORD0) : SV_Target
{
    // Site 1: the `_Texture` type's own single-index initializer.
    //CHECK: error[E41400]
    //CHECK: cannot be initialized directly from a single ResourceDescriptorHeap index{{.*}}.Handle(uint2(resourceIndex, samplerIndex))
    Sampler3D volume = ResourceDescriptorHeap[11];

    // Site 2: the `.Handle` (DescriptorHandle<T>) single-index initializer.
    //CHECK: error[E41400]
    //CHECK: its .Handle cannot be built from a single ResourceDescriptorHeap index{{.*}}.Handle(uint2(resourceIndex, samplerIndex))
    Sampler3D.Handle handle = ResourceDescriptorHeap[12];
    Sampler3D volume2 = handle;

    return volume.SampleLevel(coordinate, 0.0) + volume2.SampleLevel(coordinate, 0.0);
}
