//TEST:SIMPLE(filecheck=CHECK): -target spirv -emit-spirv-via-glsl -entry computeMain -profile glsl_450 -stage compute
//DISABLED_TEST:SIMPLE(filecheck=CHECK): -target spirv -emit-spirv-directly -entry computeMain -profile glsl_450 -stage compute

Texture2DArray t2dArray;
RWStructuredBuffer<float4> result;
SamplerComparisonState s;

// CHECK: OpSourceExtension "GL_EXT_texture_shadow_lod"
// CHECK: %{{[0-9]*}} = OpImageSampleDrefExplicitLod %float %{{[0-9]*}} %{{[0-9]*}} %float_0_5 Lod|ConstOffset %float_0 %{{[0-9]*}}

[numthreads(1,1,1)]
void computeMain()
{
    result[0] = t2dArray.SampleCmpLevelZero(s, float3(1, 2, 3), 0.5, int2(0, 0));
}