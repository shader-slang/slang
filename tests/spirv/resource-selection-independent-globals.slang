//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -stage fragment -entry main -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -stage fragment -entry main -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=GLSL): -target glsl -stage fragment -entry main -validate-ir -validate-ir-detailed

// This is the example from issue #14101. Each conditional selects an independently declared
// binding. We require both resource operations to survive legalization, with control flow
// around their uses rather than a function-local texture, sampler, or typed buffer.
// SPIRV-DAG: OpBranchConditional
// SPIRV-DAG: OpImageSampleExplicitLod
// SPIRV-DAG: OpImageFetch
// GLSL: textureLod
// GLSL: texelFetch

Texture2D<float4> textureA;
Texture2D<float4> textureB;
SamplerState samplerA;
SamplerState samplerB;
Buffer<float4> bufferA;
Buffer<float4> bufferB;

float4 main(float4 position : SV_Position) : SV_Target
{
    bool chooseA = position.x > 10;
    Texture2D<float4> texture = chooseA ? textureA : textureB;
    SamplerState sampler = chooseA ? samplerA : samplerB;
    Buffer<float4> buffer = chooseA ? bufferA : bufferB;
    return texture.SampleLevel(sampler, position.xy, 0) + buffer.Load(0);
}
