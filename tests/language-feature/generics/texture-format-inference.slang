//TEST:SIMPLE: -target hlsl -entry computeMain -stage compute -o texture-format-inference.hlsl
//TEST:SIMPLE: -target spirv -entry computeMain -stage compute -o texture-format-inference.spv
//TEST:SIMPLE: -target metal -entry computeMain -stage compute -o texture-format-inference.metal

// The texture argument must determine T directly, including in an outer generic
// function. Its operations still return the payload described by T.DataType.
T.DataType loadTexture<T : ITexelFormat>(Texture2D<T> texture)
{
    return texture.mips[0][int2(0)];
}

T.DataType forwardTexture<T : ITexelFormat>(Texture2D<T> texture)
{
    return loadTexture(texture);
}

T.DataType loadCombinedTexture<T : ITexelFormat>(Texture2D<T> texture, SamplerState sampler)
{
    __target_switch
    {
    case spirv:
        // These internal image-composition helpers are used by the SPIR-V backend.
        return loadTexture(__extractImage(__makeCombinedTextureSampler(texture, sampler)));
    default:
        return loadTexture(texture);
    }
}

void copyTexture<T : ITexelFormat>(RWTexture2D<T> texture)
{
    T.DataType value = texture[int2(0)];
    texture[int2(0)] = value;
}

T.DataType loadSample<T : ITexelFormat>(Texture2DMS<T> texture)
{
    return texture.sample[0][int2(0)];
}

T.DataType loadBuffer<T : ITexelFormat>(Buffer<T> buffer)
{
    return buffer[0];
}

void copyBuffer<T : ITexelFormat>(RWBuffer<T> buffer)
{
    T.DataType value = buffer[0];
    buffer[0] = value;
}

// Existing generic texture code only knows ITexelElement. Keep its scalar member,
// component count, scalar constructor, and arithmetic on inferred load results.
T loadLegacyTexture<T>(Texture2D<T> texture, T.Element scalar)
    where T : ITexelElement, IArithmetic
{
    let value = texture.Load(int3(0));
    return value + T(scalar);
}

int legacyElementCount<T : ITexelElement>(T value)
{
    return T.elementCount;
}

Texture2D<T, 0, 0> explicitUnknownFormat<T : ITexelElement>(Texture2D<T> texture)
{
    return texture;
}

Texture2D<T> implicitUnknownFormat<T : ITexelElement>(Texture2D<T, 0, 0> texture)
{
    return texture;
}

// These format arguments were public before named descriptors existed. Inference
// must still recover both the element and format from an ordinary texture/buffer.
T loadLegacyFormat<T : ITexelElement, let format : int>(Texture2D<T, 0, format> texture)
{
    return texture.Load(int3(0));
}

void copyLegacyFormat<T : ITexelElement, let format : int>(
    Buffer<T, format> input,
    RWBuffer<T, format> output,
    RWTexture2D<T, 0, format> texture)
{
    output[0] = input[0];
    texture[int2(0)] = output[0];
}

Texture2D<float1> oneChannel;
Texture2D<float4> fourChannels;
Texture2D<RGBA16F> formatted16;
Texture2D<RGBA32F> formatted32;
Texture2DMS<RGBA16F> multisampled;
SamplerState sampler;
RWTexture2D<float4> plainOutput;
RWTexture2D<RGBA16F> formattedOutput;
Buffer<float4> plainBuffer;
Buffer<RGBA32F> formattedBuffer;
RWBuffer<RGBA32F> outputBuffer;
Texture2D<float4, 0, 1> legacyTexture;
Buffer<float4, 1> legacyInputBuffer;
RWBuffer<float4, 1> legacyOutputBuffer;
RWTexture2D<float4, 0, 1> legacyOutputTexture;

[numthreads(1, 1, 1)]
void computeMain()
{
    float1 singleValue = loadTexture(oneChannel);
    float4 value = loadTexture(fourChannels);
    value += forwardTexture(formatted16);
    value += forwardTexture(formatted32);
    value += loadCombinedTexture(formatted16, sampler);
    value += loadSample(multisampled);
    value += loadBuffer(plainBuffer);
    value += loadBuffer(formattedBuffer);
    copyTexture(plainOutput);
    copyTexture(formattedOutput);
    copyBuffer(outputBuffer);
    value += loadLegacyTexture(implicitUnknownFormat(explicitUnknownFormat(fourChannels)), 1.0f);
    value += legacyElementCount(value);
    value += loadLegacyFormat(legacyTexture);
    copyLegacyFormat(legacyInputBuffer, legacyOutputBuffer, legacyOutputTexture);
    formattedOutput[int2(1)] = value + singleValue.x;
}
