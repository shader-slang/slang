//TEST:SIMPLE(filecheck=MTL): -target metal
//TEST:SIMPLE(filecheck=MTLLIB): -target metallib
//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=PTX): -target ptx -entry computeMain -stage compute 
//TEST:SIMPLE(filecheck=SPV_GLSL): -target spirv -entry computeMain -stage compute -emit-spirv-via-glsl
//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry computeMain -profile cs_6_6
//TEST:SIMPLE(filecheck=DXIL): -target dxil -entry computeMain -profile cs_6_6
//TEST:SIMPLE(filecheck=SPV): -target spirv
//TEST:SIMPLE(filecheck=CPP): -target cpp -entry computeMain -stage compute

// SPV_GLSL-DAG: %[[REG_GLSL:[A-Za-z_0-9]+]] = OpAccessChain{{.*}}NonUniform
// SPV_GLSL-DAG: %[[TEX_GLSL:[A-Za-z_0-9]+]] = OpLoad {{.*}} %[[REG_GLSL]]
// SPV_GLSL-DAG: OpImageSampleExplicitLod {{.*}} %[[TEX_GLSL]]

// SPV-DAG: OpDecorate %[[NU_IDX:[A-Za-z_0-9]+]] NonUniform
// SPV-DAG: %[[REG:[A-Za-z_0-9]+]] = OpAccessChain %{{.*}} %{{.*}} %[[NU_IDX]]
// SPV-DAG: %[[TEX:[A-Za-z_0-9]+]] = OpLoad {{.*}} %[[REG]]
// SPV-DAG: OpImageSampleExplicitLod {{.*}} %[[TEX]]

// HLSL: ResourceDescriptorHeap[NonUniformResourceIndex
// HLSL: SamplerDescriptorHeap[NonUniformResourceIndex(

// DXIL: define void @computeMain()

// GLSL: _slang_resource_heap{{[0-9_]+}}[nonuniformEXT

// MTL: struct Sampler2D_0
// MTL-NEXT: {
// MTL-NEXT: texture2d<float, access::sample> texture_0;
// MTL-NEXT: sampler sampler_0;
// MTL: void computeMain({{.*}} Sampler2D_0 device* t_1 {{\[\[}}buffer(0){{\]\]}})
// MTLLIB: @computeMain

// CUDA: tex2DLod
// PTX: computeMain

// CPP: struct Sampler2D_0
// CPP-NEXT: {
// CPP-NEXT: Texture2D<Vector<float, 4> > texture_0;
// CPP-NEXT: SamplerState sampler_0;
// CPP: StructuredBuffer<Sampler2D_0> t_0;

uniform StructuredBuffer<Sampler2D.Handle> t;
uniform RWStructuredBuffer<float4>.Handle buffer;

[numthreads(4,1,1)]
void computeMain(int3 id : SV_DispatchThreadID)
{
    (*buffer)[0] = nonuniform(t[id.x]).SampleLevel(float2(0.0), 0.0);
}