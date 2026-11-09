// Control cases for shader-slang/slang#13085: the explicit two-index combined handle
// `Sampler3D.Handle(uint2(...))` and separate non-combined `Texture3D`/`SamplerState` descriptors are
// valid, because each supplies every heap index it needs; both must compile.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -stage fragment -entry main -profile spirv_1_6 -capability spvDescriptorHeapEXT -emit-spirv-directly

[shader("fragment")]
float4 main(float3 coordinate : TEXCOORD0) : SV_Target
{
    Sampler3D volume = Sampler3D.Handle(uint2(11, 5));
    float4 a = volume.SampleLevel(coordinate, 0.0);

    Texture3D<float4> tex = ResourceDescriptorHeap[11];
    SamplerState samp = SamplerDescriptorHeap[7];
    float4 b = tex.SampleLevel(samp, coordinate, 0.0);

    // The legitimate forms must load the requested heap slots: resource-heap index 11, and
    // sampler-heap indices 5 (explicit combined) and 7 (separate sampler).
    //CHECK-DAG: OpUntypedAccessChainKHR{{.*}}slang_resourceHeap{{.*}}%uint_11
    //CHECK-DAG: OpUntypedAccessChainKHR{{.*}}slang_samplerHeap{{.*}}%uint_5
    //CHECK-DAG: OpUntypedAccessChainKHR{{.*}}slang_samplerHeap{{.*}}%uint_7
    return a + b;
}
