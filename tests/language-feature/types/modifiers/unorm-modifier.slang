// unorm-modifier.slang

//TEST:COMPARE_HLSL:-profile cs_5_0 -entry main
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -profile cs_5_0 -entry main
// HLSL-DAG: Buffer<unorm float4 {{ *}}> input_0
// HLSL-DAG: Texture2D<unorm float4 {{ *}}> inputTexture_0

//TEST:SIMPLE(filecheck=GENERIC-HLSL): -target hlsl -entry genericMain -stage compute
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -entry genericMain -stage compute

// Check that this compiler for glsl (without using the modifiers)
//TEST:SIMPLE(filecheck=CHECK):-target glsl -entry main -stage compute
// CHECK: uniform textureBuffer input_0;
// CHECK: texelFetch((input_0)

#ifndef __SLANG__
#define input input_0
#define inputTexture inputTexture_0
#define output output_0
#endif


Buffer<unorm float4> input;
Texture2D<unorm float4> inputTexture;
RWStructuredBuffer<float4> output;

[numthreads(4, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint tid = dispatchThreadID.x;
    output[tid] = input[tid] + inputTexture.Load(int3(dispatchThreadID.xy, 0));
}

#ifdef __SLANG__
// Resource qualifiers must survive when a generic field's texel argument becomes
// known only after specialization. Keep this Slang-only coverage separate from
// the HLSL-compatible entry point used by the FXC comparison above.
struct Resources<T : ITexelElement>
{
    Buffer<T> inputBuffer;
    Texture2D<T> inputTexture;
    RWBuffer<T> outputBuffer;
    RWTexture2D<T> outputTexture;
    // Check the associated type itself, independently of texture lowering.
    StructuredBuffer<T.DataType> projectedData;
}

// An outer no_diff modifier must not hide the normalization on the aliased type.
typealias UNorm4 = unorm float4;
Resources<no_diff UNorm4> unsignedResources;
Buffer<no_diff UNorm4> nestedBuffer;

// Looking up DataType through the format interface must retain the same qualifiers.
struct FormatPayload<T : ITexelFormat>
{
    StructuredBuffer<T.DataType> buffer;
}
FormatPayload<UNorm4> formatPayload;

// Direct resource declarations also infer a format before generic specialization.
typealias UNorm1 = unorm float;
RWTexture2D<no_diff UNorm1> directTexture;

// Gather must form a vector of plain scalar elements from a normalized texture.
Texture2D<unorm float> scalarTexture;
SamplerState textureSampler;

// Preserve the existing projection behavior: generic scalar Element retains the
// normalization, while direct Element lookup selects the nominal scalar declaration.
struct ScalarPayload<T : __BuiltinArithmeticType>
{
    StructuredBuffer<T.Element> buffer;
}
ScalarPayload<UNorm1> genericScalarPayload;
StructuredBuffer<UNorm1.Element> directScalarPayload;

// Ordinary user extensions keep their existing unqualified generic argument.
interface IPlainScalar
{
    associatedtype Scalar : __BuiltinArithmeticType;
}
extension<T : __BuiltinArithmeticType> T : IPlainScalar
{
    typealias Scalar = T;
}
StructuredBuffer<vector<UNorm1.Scalar, 4>> legacyScalarPayload;

interface ILegacyIdentity
{
    associatedtype Payload;
}
extension<T : IDifferentiable> T : ILegacyIdentity
{
    typealias Payload = T;
}
typealias NonDifferentiable = no_diff float;
struct RequiresDifferentiable<T : IDifferentiable> {}
void checkLegacyIdentity(RequiresDifferentiable<NonDifferentiable.Payload> value) {}

// Extension inference and ordinary specialization must use the same conformance witness.
struct WitnessKey<T : ITexelElement> {}
interface IQualifiedWitnessKey
{
    associatedtype Key;
}
__preserve_type_modifiers
extension<T : ITexelElement> T : IQualifiedWitnessKey
{
    typealias Key = WitnessKey<T>;
}
void checkWitnessKey(WitnessKey<UNorm4> value) {}

// GENERIC-HLSL-DAG: Buffer<unorm float4 {{ *}}> unsignedResources_inputBuffer
// GENERIC-HLSL-DAG: Texture2D<unorm float4 {{ *}}> unsignedResources_inputTexture
// GENERIC-HLSL-DAG: RWBuffer<unorm float4 {{ *}}> unsignedResources_outputBuffer
// GENERIC-HLSL-DAG: RWTexture2D<unorm float4 {{ *}}> unsignedResources_outputTexture
// GENERIC-HLSL-DAG: StructuredBuffer<unorm float4 {{ *}}> unsignedResources_projectedData
// GENERIC-HLSL-DAG: StructuredBuffer<unorm float4 {{ *}}> formatPayload_buffer
// GENERIC-HLSL-DAG: Buffer<unorm float4 {{ *}}> nestedBuffer
// GENERIC-HLSL-DAG: RWTexture2D<unorm float {{ *}}> directTexture
// GENERIC-HLSL-DAG: Texture2D<unorm float {{ *}}> scalarTexture
// GENERIC-HLSL-DAG: StructuredBuffer<unorm float {{ *}}> genericScalarPayload_buffer
// GENERIC-HLSL-DAG: StructuredBuffer<float {{ *}}> directScalarPayload
// GENERIC-HLSL-DAG: StructuredBuffer<float4 {{ *}}> legacyScalarPayload
// SPIRV-DAG: OpTypeImage %float 2D 2 0 0 2 Rgba8{{ *$}}
// SPIRV-DAG: OpTypeImage %float Buffer 2 0 0 2 Rgba8{{ *$}}
// SPIRV-DAG: OpTypeImage %float 2D 2 0 0 2 R8{{ *$}}
// SPIRV-DAG: OpImageGather

void copyTexture<T : ITexelElement>(Texture2D<T> input, RWTexture2D<T> output)
{
    output[uint2(0)] = input.Load(int3(0));
}

[numthreads(1, 1, 1)]
void genericMain()
{
    UNorm4.Key key = {};
    checkWitnessKey(key);
    copyTexture(unsignedResources.inputTexture, unsignedResources.outputTexture);
    unsignedResources.outputBuffer[0] = unsignedResources.inputBuffer[0] + nestedBuffer[0]
        + unsignedResources.projectedData[0] + formatPayload.buffer[0];
    directTexture[uint2(0)] = 0.5;
    output[0] = scalarTexture.Gather(textureSampler, float2(0.5))
        + genericScalarPayload.buffer[0] + directScalarPayload[0] + legacyScalarPayload[0];
}
#endif
