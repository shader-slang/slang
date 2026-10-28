// snorm-modifier.slang

//TEST:COMPARE_HLSL:-profile cs_5_0 -entry main
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -profile cs_5_0 -entry main
// HLSL-DAG: Buffer<snorm float4 {{ *}}> input_0
// HLSL-DAG: Texture2D<snorm float4 {{ *}}> inputTexture_0

//TEST:SIMPLE(filecheck=GENERIC-HLSL): -target hlsl -entry genericMain -stage compute
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -entry genericMain -stage compute

#ifndef __SLANG__
#define input input_0
#define inputTexture inputTexture_0
#define output output_0
#endif

Buffer<snorm float4> input;
Texture2D<snorm float4> inputTexture;
RWStructuredBuffer<float4> output;

[numthreads(4, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;
    output[tid] = input[tid] + inputTexture.Load(int3(dispatchThreadID.xy, 0));
}

#ifdef __SLANG__
// Generic resource fields must retain their normalized formats after specialization.
struct Resources<T : ITexelElement>
{
    Buffer<T> inputBuffer;
    Texture2D<T> inputTexture;
    RWBuffer<T> outputBuffer;
    RWTexture2D<T> outputTexture;
    // Check the associated type itself, independently of texture lowering.
    StructuredBuffer<T.DataType> projectedData;
}
typealias SNorm4 = snorm float4;
Resources<SNorm4> signedResources;

// Looking up DataType through the format interface must retain the same qualifiers.
struct FormatPayload<T : ITexelFormat>
{
    StructuredBuffer<T.DataType> buffer;
}
FormatPayload<SNorm4> formatPayload;

// Gather must form a vector of plain scalar elements from a normalized texture.
Texture2D<snorm float> scalarTexture;
SamplerState textureSampler;

// Preserve the existing projection behavior: generic scalar Element retains the
// normalization, while direct Element lookup selects the nominal scalar declaration.
struct ScalarPayload<T : __BuiltinArithmeticType>
{
    StructuredBuffer<T.Element> buffer;
}
typealias SNorm1 = snorm float;
ScalarPayload<SNorm1> genericScalarPayload;
StructuredBuffer<SNorm1.Element> directScalarPayload;

// Ordinary user extensions keep their existing unqualified generic argument.
interface IPlainScalar
{
    associatedtype Scalar : __BuiltinArithmeticType;
}
extension<T : __BuiltinArithmeticType> T : IPlainScalar
{
    typealias Scalar = T;
}
StructuredBuffer<vector<SNorm1.Scalar, 4>> legacyScalarPayload;

// The built-in texel-format adapter must preserve payload qualifiers through
// direct DataType lookup and late specialization.
struct WitnessKey<T : ITexelElement> {}
void checkWitnessKey(WitnessKey<SNorm1> value) {}

WitnessKey<T.DataType> getElementDataTypeKey<T : ITexelElement>()
{
    return {};
}

WitnessKey<T.DataType> getFormatDataTypeKey<T : ITexelFormat>()
{
    return {};
}

// Exact type equality also checks that generic specialization and direct lookup
// use the same ITexelFormat witness. Passing values can ignore hidden witnesses.
struct FormatWitnessKey<T : ITexelFormat> {}
struct FormatKeyFromElement<T : ITexelElement>
{
    typealias Key = FormatWitnessKey<T>;
}
struct RequireSame<A, B> where A == B {}
RequireSame<FormatKeyFromElement<SNorm1>.Key, FormatWitnessKey<SNorm1>> formatWitnessKey;

// GENERIC-HLSL-DAG: Buffer<snorm float4 {{ *}}> signedResources_inputBuffer
// GENERIC-HLSL-DAG: Texture2D<snorm float4 {{ *}}> signedResources_inputTexture
// GENERIC-HLSL-DAG: RWBuffer<snorm float4 {{ *}}> signedResources_outputBuffer
// GENERIC-HLSL-DAG: RWTexture2D<snorm float4 {{ *}}> signedResources_outputTexture
// GENERIC-HLSL-DAG: StructuredBuffer<snorm float4 {{ *}}> signedResources_projectedData
// GENERIC-HLSL-DAG: StructuredBuffer<snorm float4 {{ *}}> formatPayload_buffer
// GENERIC-HLSL-DAG: Texture2D<snorm float {{ *}}> scalarTexture
// GENERIC-HLSL-DAG: StructuredBuffer<snorm float {{ *}}> genericScalarPayload_buffer
// GENERIC-HLSL-DAG: StructuredBuffer<float {{ *}}> directScalarPayload
// GENERIC-HLSL-DAG: StructuredBuffer<float4 {{ *}}> legacyScalarPayload
// SPIRV-DAG: OpTypeImage %float 2D 2 0 0 2 Rgba8Snorm{{ *$}}
// SPIRV-DAG: OpTypeImage %float Buffer 2 0 0 2 Rgba8Snorm{{ *$}}
// SPIRV-DAG: OpImageGather

void copyTexture<T : ITexelElement>(Texture2D<T> input, RWTexture2D<T> output)
{
    output[uint2(0)] = input.Load(int3(0));
}

[numthreads(1, 1, 1)]
void genericMain()
{
    WitnessKey<SNorm1.DataType> key = {};
    checkWitnessKey(key);
    checkWitnessKey(getElementDataTypeKey<SNorm1>());
    checkWitnessKey(getFormatDataTypeKey<SNorm1>());
    copyTexture(signedResources.inputTexture, signedResources.outputTexture);
    signedResources.outputBuffer[0] = signedResources.inputBuffer[0]
        + signedResources.projectedData[0] + formatPayload.buffer[0];
    output[0] = scalarTexture.Gather(textureSampler, float2(0.5))
        + genericScalarPayload.buffer[0] + directScalarPayload[0] + legacyScalarPayload[0];
}
#endif
