// Test GLSL output for SampleCmpBias and SampleCmpGrad functions
//TEST:SIMPLE(filecheck=CHECK_GLSL): -stage fragment -entry fragmentMain -target glsl -allow-glsl

sampler1DShadow shadowSampler1D;
sampler2DShadow shadowSampler2D;
samplerCubeShadow shadowSamplerCube;
sampler1DArrayShadow shadowSampler1DArray;
sampler2DArrayShadow shadowSampler2DArray;

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

    float bias = 0.5;
    float shadow = 0;

    // Test SampleCmpBias - should generate texture() with bias parameter
    //CHECK_GLSL: texture(shadowSampler1D
    shadow += shadowSampler1D.SampleCmpBias(input.texCoord.x, input.depth, bias);
    
    //CHECK_GLSL: textureOffset(shadowSampler1D
    shadow += shadowSampler1D.SampleCmpBias(input.texCoord.x, input.depth, bias, offset1);

    //CHECK_GLSL: texture(shadowSampler2D
    shadow += shadowSampler2D.SampleCmpBias(input.texCoord, input.depth, bias);
    
    //CHECK_GLSL: textureOffset(shadowSampler2D
    shadow += shadowSampler2D.SampleCmpBias(input.texCoord, input.depth, bias, offset2);

    //CHECK_GLSL: texture(shadowSamplerCube
    shadow += shadowSamplerCube.SampleCmpBias(float3(input.texCoord, 1.0), input.depth, bias);

    // Test SampleCmpGrad - should generate textureGrad() calls
    //CHECK_GLSL: textureGrad(shadowSampler1D
    shadow += shadowSampler1D.SampleCmpGrad(input.texCoord.x, input.depth, ddx1, ddy1);
    
    //CHECK_GLSL: textureGradOffset(shadowSampler1D
    shadow += shadowSampler1D.SampleCmpGrad(input.texCoord.x, input.depth, ddx1, ddy1, offset1);

    //CHECK_GLSL: textureGrad(shadowSampler2D
    shadow += shadowSampler2D.SampleCmpGrad(input.texCoord, input.depth, ddx2, ddy2);
    
    //CHECK_GLSL: textureGradOffset(shadowSampler2D
    shadow += shadowSampler2D.SampleCmpGrad(input.texCoord, input.depth, ddx2, ddy2, offset2);

    //CHECK_GLSL: textureGrad(shadowSamplerCube
    shadow += shadowSamplerCube.SampleCmpGrad(float3(input.texCoord, 1.0), input.depth, ddx3, ddy3);

    //CHECK_GLSL: textureGrad(shadowSampler1DArray
    shadow += shadowSampler1DArray.SampleCmpGrad(float2(input.texCoord.x, 0), input.depth, ddx1, ddy1);

    //CHECK_GLSL: textureGrad(shadowSampler2DArray
    shadow += shadowSampler2DArray.SampleCmpGrad(float3(input.texCoord, 0), input.depth, ddx2, ddy2);

    float finalShadow = shadow / 13.0;

    output.color = float4(finalShadow, finalShadow, finalShadow, 1.0);
    return output;
}
