// Control cases for shader-slang/slang#13085: descriptor-heap forms that legitimately supply the
// indices they need must keep compiling.
//   (1) The explicit two-index combined form `Sampler3D.Handle(uint2(resourceIndex, samplerIndex))`.
//   (2) Separate, non-combined `Texture3D` + `SamplerState`, each carrying a single index from its
//       own descriptor heap; neither is a combined texture-sampler.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -stage fragment -entry main -profile spirv_1_6 -capability spvDescriptorHeapEXT -emit-spirv-directly

[shader("fragment")]
float4 main(float3 coordinate : TEXCOORD0) : SV_Target
{
    Sampler3D volume = Sampler3D.Handle(uint2(11, 5));
    float4 a = volume.SampleLevel(coordinate, 0.0);

    Texture3D<float4> tex = ResourceDescriptorHeap[11];
    SamplerState samp = SamplerDescriptorHeap[7];
    float4 b = tex.SampleLevel(samp, coordinate, 0.0);

    //CHECK: OpEntryPoint
    return a + b;
}
