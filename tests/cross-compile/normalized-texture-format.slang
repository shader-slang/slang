//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -entry computeMain -stage compute

// Resolving a resource element type must preserve the texel's normalization
// modifier, including when the qualifier becomes known only after specializing
// a generic resource field. It affects HLSL declarations and SPIR-V formats.
struct Resources<T : ITexelElement>
{
    Buffer<T> inputBuffer;
    Texture2D<T> inputTexture;
    RWBuffer<T> outputBuffer;
    RWTexture2D<T> outputTexture;
}
// An outer no_diff modifier must not hide the normalization on the aliased type.
typealias UNorm4 = unorm float4;
Resources<no_diff UNorm4> unsignedResources;
Resources<snorm float4> signedResources;
Buffer<no_diff UNorm4> nestedBuffer;

// Direct resource declarations also infer a format before generic specialization.
typealias UNorm1 = unorm float;
RWTexture2D<no_diff UNorm1> directTexture;

// HLSL-DAG: Buffer<unorm float4 {{ *}}> unsignedResources_inputBuffer
// HLSL-DAG: Texture2D<unorm float4 {{ *}}> unsignedResources_inputTexture
// HLSL-DAG: RWBuffer<unorm float4 {{ *}}> unsignedResources_outputBuffer
// HLSL-DAG: RWTexture2D<unorm float4 {{ *}}> unsignedResources_outputTexture
// HLSL-DAG: Buffer<snorm float4 {{ *}}> signedResources_inputBuffer
// HLSL-DAG: Texture2D<snorm float4 {{ *}}> signedResources_inputTexture
// HLSL-DAG: RWBuffer<snorm float4 {{ *}}> signedResources_outputBuffer
// HLSL-DAG: RWTexture2D<snorm float4 {{ *}}> signedResources_outputTexture
// HLSL-DAG: Buffer<unorm float4 {{ *}}> nestedBuffer
// HLSL-DAG: RWTexture2D<unorm float {{ *}}> directTexture
// SPIRV-DAG: OpTypeImage %float 2D 2 0 0 2 Rgba8{{ *$}}
// SPIRV-DAG: OpTypeImage %float 2D 2 0 0 2 Rgba8Snorm{{ *$}}
// SPIRV-DAG: OpTypeImage %float Buffer 2 0 0 2 Rgba8{{ *$}}
// SPIRV-DAG: OpTypeImage %float Buffer 2 0 0 2 Rgba8Snorm{{ *$}}
// SPIRV-DAG: OpTypeImage %float 2D 2 0 0 2 R8{{ *$}}

void copyTexture<T : ITexelElement>(Texture2D<T> input, RWTexture2D<T> output)
{
    output[uint2(0)] = input.Load(int3(0));
}

[numthreads(1, 1, 1)]
void computeMain()
{
    copyTexture(unsignedResources.inputTexture, unsignedResources.outputTexture);
    copyTexture(signedResources.inputTexture, signedResources.outputTexture);
    unsignedResources.outputBuffer[0] = unsignedResources.inputBuffer[0] + nestedBuffer[0];
    signedResources.outputBuffer[0] = signedResources.inputBuffer[0];
    directTexture[uint2(0)] = 0.5;
}
