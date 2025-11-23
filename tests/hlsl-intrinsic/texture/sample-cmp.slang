// Test shader for SampleCmpGrad and SampleCmpBias functions
//TEST:SIMPLE(filecheck=CHECK_HLSL): -stage fragment -entry fragmentMain -target hlsl -DUSE_SPARSE
//TEST:SIMPLE(filecheck=CHECK_DXIL): -target dxil-assembly -entry fragmentMain -stage fragment -profile ps_6_8 -DUSE_SPARSE
//TEST:SIMPLE(filecheck=CHECK_SPIRV): -stage fragment -entry fragmentMain -target spirv -DUSE_SPARSE

SamplerComparisonState shadowSampler;
Texture1D<float> shadowMap1D;
Texture1DArray<float> shadowMap1DArray;
Texture2D<float> shadowMap2D;
Texture2DArray<float> shadowMap2DArray;
TextureCube<float> shadowMapCube;
TextureCubeArray<float> shadowMapCubeArray;

Sampler1DShadow sampler1DShadow;
Sampler1DArrayShadow sampler1DArrayShadow;
Sampler2DShadow sampler2DShadow;
Sampler2DArrayShadow sampler2DArrayShadow;
SamplerCubeShadow samplerCubeShadow;
SamplerCubeArrayShadow samplerCubeArrayShadow;

struct PSInput
{
    float4 position : SV_Position;
    float2 texCoord : TEXCOORD0;
    float depth : DEPTH;
};

struct PSOutput
{
    float4 color : SV_Target0;
};

PSOutput fragmentMain(PSInput input)
{
    PSOutput output;

    float ddx1 = 0.001;
    float ddy1 = 0.001;
    float2 ddx2 = float2(0.001, 0.0);
    float2 ddy2 = float2(0.0, 0.001);
    float3 ddx3 = float3(0.001, 0.0, 0.0);
    float3 ddy3 = float3(0.0, 0.001, 0.0);

    int offset1 = 1;
    int2 offset2 = int2(1, 0);
    int3 offset3 = int3(1, 0, 0);

    float bias = 0.5;
    float shadow = 0;
#ifdef USE_SPARSE
    uint status;
#endif

    //CHECK_HLSL: SampleCmpGrad
    //CHECK_DXIL: dx.op.sampleCmpGrad.f32
    //CHECK_SPIRV: OpImageSampleDrefExplicitLod {{.*}} Grad
    shadow += shadowMap1D.SampleCmpGrad(shadowSampler, input.texCoord.x, input.depth, ddx1, ddy1);
    //CHECK_HLSL: SampleCmpGrad
    //CHECK_DXIL: dx.op.sampleCmpGrad.f32
    //CHECK_SPIRV: OpImageSampleDrefExplicitLod {{.*}} Grad|ConstOffset
    shadow += shadowMap1D.SampleCmpGrad(shadowSampler, input.texCoord.x, input.depth, ddx1, ddy1, offset1);
#ifdef USE_SPARSE
    //CHECK_HLSL: SampleCmpGrad
    //CHECK_DXIL: dx.op.sampleCmpGrad.f32
    //CHECK_SPIRV: OpImageSparseSampleDrefExplicitLod {{.*}} Grad|ConstOffset
    shadow += shadowMap1D.SampleCmpGrad(shadowSampler, input.texCoord.x, input.depth, ddx1, ddy1, offset1, status);
#endif

    //CHECK_HLSL: SampleCmpGrad
    //CHECK_DXIL: dx.op.sampleCmpGrad.f32
    //CHECK_SPIRV: OpImageSampleDrefExplicitLod {{.*}} Grad
    shadow += sampler1DShadow.SampleCmpGrad(input.texCoord.x, input.depth, ddx1, ddy1);
    //CHECK_HLSL: SampleCmpGrad
    //CHECK_DXIL: dx.op.sampleCmpGrad.f32
    //CHECK_SPIRV: OpImageSampleDrefExplicitLod {{.*}} Grad|ConstOffset
    shadow += sampler1DShadow.SampleCmpGrad(input.texCoord.x, input.depth, ddx1, ddy1, offset1);
#ifdef USE_SPARSE
    //CHECK_HLSL: SampleCmpGrad
    //CHECK_DXIL: dx.op.sampleCmpGrad.f32
    //CHECK_SPIRV: OpImageSparseSampleDrefExplicitLod {{.*}} Grad|ConstOffset
    shadow += sampler1DShadow.SampleCmpGrad(input.texCoord.x, input.depth, ddx1, ddy1, offset1, status);
#endif

    //CHECK_HLSL: SampleCmpBias
    //CHECK_DXIL: dx.op.sampleCmpBias.f32
    //CHECK_SPIRV: OpImageSampleDrefImplicitLod {{.*}} Bias
    shadow += shadowMap1D.SampleCmpBias(shadowSampler, input.texCoord.x, input.depth, bias);

    //CHECK_HLSL: SampleCmpBias
    //CHECK_DXIL: dx.op.sampleCmpBias.f32
    //CHECK_SPIRV: OpImageSampleDrefImplicitLod {{.*}} Bias|ConstOffset
    shadow += shadowMap1D.SampleCmpBias(shadowSampler, input.texCoord.x, input.depth, bias, offset1);
#ifdef USE_SPARSE
    //CHECK_HLSL: SampleCmpBias
    //CHECK_DXIL: dx.op.sampleCmpBias.f32
    //CHECK_SPIRV: OpImageSparseSampleDrefImplicitLod {{.*}} Bias|ConstOffset
    shadow += shadowMap1D.SampleCmpBias(shadowSampler, input.texCoord.x, input.depth, bias, offset1, status);
#endif

    //CHECK_HLSL: SampleCmpBias
    //CHECK_DXIL: dx.op.sampleCmpBias.f32
    //CHECK_SPIRV: OpImageSampleDrefImplicitLod {{.*}} Bias
    shadow += sampler1DShadow.SampleCmpBias(input.texCoord.x, input.depth, bias);
    //CHECK_HLSL: SampleCmpBias
    //CHECK_DXIL: dx.op.sampleCmpBias.f32
    //CHECK_SPIRV: OpImageSampleDrefImplicitLod {{.*}} Bias|ConstOffset
    shadow += sampler1DShadow.SampleCmpBias(input.texCoord.x, input.depth, bias, offset1);
#ifdef USE_SPARSE
    //CHECK_HLSL: SampleCmpBias
    //CHECK_DXIL: dx.op.sampleCmpBias.f32
    //CHECK_SPIRV: OpImageSparseSampleDrefImplicitLod {{.*}} Bias|ConstOffset
    shadow += sampler1DShadow.SampleCmpBias(input.texCoord.x, input.depth, bias, offset1, status);
#endif

    // ===== Texture1DArray Tests =====
    float2 texCoord1DArray = float2(input.texCoord.x, 0);

    //CHECK_HLSL: SampleCmpGrad
    //CHECK_DXIL: dx.op.sampleCmpGrad.f32
    //CHECK_SPIRV: OpImageSampleDrefExplicitLod {{.*}} Grad
    shadow += shadowMap1DArray.SampleCmpGrad(shadowSampler, texCoord1DArray, input.depth, ddx1, ddy1);
    //CHECK_HLSL: SampleCmpGrad
    //CHECK_DXIL: dx.op.sampleCmpGrad.f32
    //CHECK_SPIRV: OpImageSampleDrefExplicitLod {{.*}} Grad|ConstOffset
    shadow += shadowMap1DArray.SampleCmpGrad(shadowSampler, texCoord1DArray, input.depth, ddx1, ddy1, offset1);
#ifdef USE_SPARSE
    //CHECK_HLSL: SampleCmpGrad
    //CHECK_DXIL: dx.op.sampleCmpGrad.f32
    //CHECK_SPIRV: OpImageSparseSampleDrefExplicitLod {{.*}} Grad|ConstOffset
    shadow += shadowMap1DArray.SampleCmpGrad(shadowSampler, texCoord1DArray, input.depth, ddx1, ddy1, offset1, status);
#endif

    //CHECK_HLSL: SampleCmpGrad
    //CHECK_DXIL: dx.op.sampleCmpGrad.f32
    //CHECK_SPIRV: OpImageSampleDrefExplicitLod {{.*}} Grad
    shadow += sampler1DArrayShadow.SampleCmpGrad(texCoord1DArray, input.depth, ddx1, ddy1);
    //CHECK_HLSL: SampleCmpGrad
    //CHECK_DXIL: dx.op.sampleCmpGrad.f32
    //CHECK_SPIRV: OpImageSampleDrefExplicitLod {{.*}} Grad|ConstOffset
    shadow += sampler1DArrayShadow.SampleCmpGrad(texCoord1DArray, input.depth, ddx1, ddy1, offset1);

#ifdef USE_SPARSE
    //CHECK_HLSL: SampleCmpGrad
    //CHECK_DXIL: dx.op.sampleCmpGrad.f32
    //CHECK_SPIRV: OpImageSparseSampleDrefExplicitLod {{.*}} Grad|ConstOffset
    shadow += sampler1DArrayShadow.SampleCmpGrad(texCoord1DArray, input.depth, ddx1, ddy1, offset1, status);
#endif

    //CHECK_HLSL: SampleCmpBias
    //CHECK_DXIL: dx.op.sampleCmpBias.f32
    //CHECK_SPIRV: OpImageSampleDrefImplicitLod {{.*}} Bias
    shadow += shadowMap1DArray.SampleCmpBias(shadowSampler, texCoord1DArray, input.depth, bias);
    //CHECK_HLSL: SampleCmpBias
    //CHECK_DXIL: dx.op.sampleCmpBias.f32
    //CHECK_SPIRV: OpImageSampleDrefImplicitLod {{.*}} Bias|ConstOffset
    shadow += shadowMap1DArray.SampleCmpBias(shadowSampler, texCoord1DArray, input.depth, bias, offset1);
#ifdef USE_SPARSE
    //CHECK_HLSL: SampleCmpBias
    //CHECK_DXIL: dx.op.sampleCmpBias.f32
    //CHECK_SPIRV: OpImageSparseSampleDrefImplicitLod {{.*}} Bias|ConstOffset
    shadow += shadowMap1DArray.SampleCmpBias(shadowSampler, texCoord1DArray, input.depth, bias, offset1, status);
#endif

    //CHECK_HLSL: SampleCmpBias
    //CHECK_DXIL: dx.op.sampleCmpBias.f32
    //CHECK_SPIRV: OpImageSampleDrefImplicitLod {{.*}} Bias
    shadow += sampler1DArrayShadow.SampleCmpBias(texCoord1DArray, input.depth, bias);
    //CHECK_HLSL: SampleCmpBias
    //CHECK_DXIL: dx.op.sampleCmpBias.f32
    //CHECK_SPIRV: OpImageSampleDrefImplicitLod {{.*}} Bias|ConstOffset
    shadow += sampler1DArrayShadow.SampleCmpBias(texCoord1DArray, input.depth, bias, offset1);
#ifdef USE_SPARSE
    //CHECK_HLSL: SampleCmpBias
    //CHECK_DXIL: dx.op.sampleCmpBias.f32
    //CHECK_SPIRV: OpImageSparseSampleDrefImplicitLod {{.*}} Bias|ConstOffset
    shadow += sampler1DArrayShadow.SampleCmpBias(texCoord1DArray, input.depth, bias, offset1, status);
#endif

    //CHECK_HLSL: SampleCmpGrad
    //CHECK_DXIL: dx.op.sampleCmpGrad.f32
    //CHECK_SPIRV: OpImageSampleDrefExplicitLod {{.*}} Grad
    shadow += shadowMap2D.SampleCmpGrad(shadowSampler, input.texCoord, input.depth, ddx2, ddy2);
    //CHECK_HLSL: SampleCmpGrad
    //CHECK_DXIL: dx.op.sampleCmpGrad.f32
    //CHECK_SPIRV: OpImageSampleDrefExplicitLod {{.*}} Grad|ConstOffset
    shadow += shadowMap2D.SampleCmpGrad(shadowSampler, input.texCoord, input.depth, ddx2, ddy2, offset2);
#ifdef USE_SPARSE
    //CHECK_HLSL: SampleCmpGrad
    //CHECK_DXIL: dx.op.sampleCmpGrad.f32
    //CHECK_SPIRV: OpImageSparseSampleDrefExplicitLod {{.*}} Grad|ConstOffset
    shadow += shadowMap2D.SampleCmpGrad(shadowSampler, input.texCoord, input.depth, ddx2, ddy2, offset2, status);
#endif

    //CHECK_HLSL: SampleCmpGrad
    //CHECK_DXIL: dx.op.sampleCmpGrad.f32
    //CHECK_SPIRV: OpImageSampleDrefExplicitLod {{.*}} Grad
    shadow += sampler2DShadow.SampleCmpGrad(input.texCoord, input.depth, ddx2, ddy2);
    //CHECK_HLSL: SampleCmpGrad
    //CHECK_DXIL: dx.op.sampleCmpGrad.f32
    //CHECK_SPIRV: OpImageSampleDrefExplicitLod {{.*}} Grad|ConstOffset
    shadow += sampler2DShadow.SampleCmpGrad(input.texCoord, input.depth, ddx2, ddy2, offset2);
#ifdef USE_SPARSE
    //CHECK_HLSL: SampleCmpGrad
    //CHECK_DXIL: dx.op.sampleCmpGrad.f32
    //CHECK_SPIRV: OpImageSparseSampleDrefExplicitLod {{.*}} Grad|ConstOffset
    shadow += sampler2DShadow.SampleCmpGrad(input.texCoord, input.depth, ddx2, ddy2, offset2, status);
#endif

    //CHECK_HLSL: SampleCmpBias
    //CHECK_DXIL: dx.op.sampleCmpBias.f32
    //CHECK_SPIRV: OpImageSampleDrefImplicitLod {{.*}} Bias
    shadow += shadowMap2D.SampleCmpBias(shadowSampler, input.texCoord, input.depth, bias);
    //CHECK_HLSL: SampleCmpBias
    //CHECK_DXIL: dx.op.sampleCmpBias.f32
    //CHECK_SPIRV: OpImageSampleDrefImplicitLod {{.*}} Bias|ConstOffset
    shadow += shadowMap2D.SampleCmpBias(shadowSampler, input.texCoord, input.depth, bias, offset2);
#ifdef USE_SPARSE
    //CHECK_HLSL: SampleCmpBias
    //CHECK_DXIL: dx.op.sampleCmpBias.f32
    //CHECK_SPIRV: OpImageSparseSampleDrefImplicitLod {{.*}} Bias|ConstOffset
    shadow += shadowMap2D.SampleCmpBias(shadowSampler, input.texCoord, input.depth, bias, offset2, status);
#endif

    //CHECK_HLSL: SampleCmpBias
    //CHECK_DXIL: dx.op.sampleCmpBias.f32
    //CHECK_SPIRV: OpImageSampleDrefImplicitLod {{.*}} Bias
    shadow += sampler2DShadow.SampleCmpBias(input.texCoord, input.depth, bias);
    //CHECK_HLSL: SampleCmpBias
    //CHECK_DXIL: dx.op.sampleCmpBias.f32
    //CHECK_SPIRV: OpImageSampleDrefImplicitLod {{.*}} Bias|ConstOffset
    shadow += sampler2DShadow.SampleCmpBias(input.texCoord, input.depth, bias, offset2);
#ifdef USE_SPARSE
    //CHECK_HLSL: SampleCmpBias
    //CHECK_DXIL: dx.op.sampleCmpBias.f32
    //CHECK_SPIRV: OpImageSparseSampleDrefImplicitLod {{.*}} Bias|ConstOffset
    shadow += sampler2DShadow.SampleCmpBias(input.texCoord, input.depth, bias, offset2, status);
#endif

    float3 texCoord2DArray = float3(input.texCoord, 0);

    //CHECK_HLSL: SampleCmpGrad
    //CHECK_DXIL: dx.op.sampleCmpGrad.f32
    //CHECK_SPIRV: OpImageSampleDrefExplicitLod {{.*}} Grad
    shadow += shadowMap2DArray.SampleCmpGrad(shadowSampler, texCoord2DArray, input.depth, ddx2, ddy2);
    //CHECK_HLSL: SampleCmpGrad
    //CHECK_DXIL: dx.op.sampleCmpGrad.f32
    //CHECK_SPIRV: OpImageSampleDrefExplicitLod {{.*}} Grad|ConstOffset
    shadow += shadowMap2DArray.SampleCmpGrad(shadowSampler, texCoord2DArray, input.depth, ddx2, ddy2, offset2);
#ifdef USE_SPARSE
    //CHECK_HLSL: SampleCmpGrad
    //CHECK_DXIL: dx.op.sampleCmpGrad.f32
    //CHECK_SPIRV: OpImageSparseSampleDrefExplicitLod {{.*}} Grad|ConstOffset
    shadow += shadowMap2DArray.SampleCmpGrad(shadowSampler, texCoord2DArray, input.depth, ddx2, ddy2, offset2, status);
#endif

    //CHECK_HLSL: SampleCmpGrad
    //CHECK_DXIL: dx.op.sampleCmpGrad.f32
    //CHECK_SPIRV: OpImageSampleDrefExplicitLod {{.*}} Grad
    shadow += sampler2DArrayShadow.SampleCmpGrad(texCoord2DArray, input.depth, ddx2, ddy2);
    //CHECK_HLSL: SampleCmpGrad
    //CHECK_DXIL: dx.op.sampleCmpGrad.f32
    //CHECK_SPIRV: OpImageSampleDrefExplicitLod {{.*}} Grad|ConstOffset
    shadow += sampler2DArrayShadow.SampleCmpGrad(texCoord2DArray, input.depth, ddx2, ddy2, offset2);
#ifdef USE_SPARSE
    //CHECK_HLSL: SampleCmpGrad
    //CHECK_DXIL: dx.op.sampleCmpGrad.f32
    //CHECK_SPIRV: OpImageSparseSampleDrefExplicitLod {{.*}} Grad|ConstOffset
    shadow += sampler2DArrayShadow.SampleCmpGrad(texCoord2DArray, input.depth, ddx2, ddy2, offset2, status);
#endif

    //CHECK_HLSL: SampleCmpBias
    //CHECK_DXIL: dx.op.sampleCmpBias.f32
    //CHECK_SPIRV: OpImageSampleDrefImplicitLod {{.*}} Bias
    shadow += shadowMap2DArray.SampleCmpBias(shadowSampler, texCoord2DArray, input.depth, bias);
    //CHECK_HLSL: SampleCmpBias
    //CHECK_DXIL: dx.op.sampleCmpBias.f32
    //CHECK_SPIRV: OpImageSampleDrefImplicitLod {{.*}} Bias|ConstOffset
    shadow += shadowMap2DArray.SampleCmpBias(shadowSampler, texCoord2DArray, input.depth, bias, offset2);
#ifdef USE_SPARSE
    //CHECK_HLSL: SampleCmpBias
    //CHECK_DXIL: dx.op.sampleCmpBias.f32
    //CHECK_SPIRV: OpImageSparseSampleDrefImplicitLod {{.*}} Bias|ConstOffset
    shadow += shadowMap2DArray.SampleCmpBias(shadowSampler, texCoord2DArray, input.depth, bias, offset2, status);
#endif

    //CHECK_HLSL: SampleCmpBias
    //CHECK_DXIL: dx.op.sampleCmpBias.f32
    //CHECK_SPIRV: OpImageSampleDrefImplicitLod {{.*}} Bias
    shadow += sampler2DArrayShadow.SampleCmpBias(texCoord2DArray, input.depth, bias);
    //CHECK_HLSL: SampleCmpBias
    //CHECK_DXIL: dx.op.sampleCmpBias.f32
    //CHECK_SPIRV: OpImageSampleDrefImplicitLod {{.*}} Bias|ConstOffset
    shadow += sampler2DArrayShadow.SampleCmpBias(texCoord2DArray, input.depth, bias, offset2);
#ifdef USE_SPARSE
    //CHECK_HLSL: SampleCmpBias
    //CHECK_DXIL: dx.op.sampleCmpBias.f32
    //CHECK_SPIRV: OpImageSparseSampleDrefImplicitLod {{.*}} Bias|ConstOffset
    shadow += sampler2DArrayShadow.SampleCmpBias(texCoord2DArray, input.depth, bias, offset2, status);
#endif

    // ===== TextureCube Tests =====
    float3 texCoordCube = float3(input.texCoord, 1.0);

    //CHECK_HLSL: SampleCmpGrad
    //CHECK_DXIL: dx.op.sampleCmpGrad.f32
    //CHECK_SPIRV: OpImageSampleDrefExplicitLod {{.*}} Grad
    shadow += shadowMapCube.SampleCmpGrad(shadowSampler, texCoordCube, input.depth, ddx3, ddy3);
    //CHECK_HLSL: SampleCmpGrad
    //CHECK_DXIL: dx.op.sampleCmpGrad.f32
    //CHECK_SPIRV: OpImageSampleDrefExplicitLod {{.*}} Grad
    shadow += samplerCubeShadow.SampleCmpGrad(texCoordCube, input.depth, ddx3, ddy3);

    //CHECK_HLSL: SampleCmpBias
    //CHECK_DXIL: dx.op.sampleCmpBias.f32
    //CHECK_SPIRV: OpImageSampleDrefImplicitLod {{.*}} Bias
    shadow += shadowMapCube.SampleCmpBias(shadowSampler, texCoordCube, input.depth, bias);
    //CHECK_HLSL: SampleCmpBias
    //CHECK_DXIL: dx.op.sampleCmpBias.f32
    //CHECK_SPIRV: OpImageSampleDrefImplicitLod {{.*}} Bias
    shadow += samplerCubeShadow.SampleCmpBias(texCoordCube, input.depth, bias);

    // ===== TextureCubeArray Tests =====
    float4 texCoordCubeArray = float4(input.texCoord, 1.0, 0);

    //CHECK_HLSL: SampleCmpBias
    //CHECK_DXIL: dx.op.sampleCmpBias.f32
    //CHECK_SPIRV: OpImageSampleDrefImplicitLod {{.*}} Bias
    shadow += shadowMapCubeArray.SampleCmpBias(shadowSampler, texCoordCubeArray, input.depth, bias);

    //CHECK_HLSL: SampleCmpBias
    //CHECK_DXIL: dx.op.sampleCmpBias.f32
    //CHECK_SPIRV: OpImageSampleDrefImplicitLod {{.*}} Bias
    shadow += samplerCubeArrayShadow.SampleCmpBias(texCoordCubeArray, input.depth, bias);

    float finalShadow = shadow / 72.0;

    output.color = float4(finalShadow, finalShadow, finalShadow, 1.0);
    return output;
}

