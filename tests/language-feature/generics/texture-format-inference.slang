//TEST:SIMPLE: -target hlsl -entry computeMain -stage compute -o texture-format-inference.hlsl
//TEST:SIMPLE: -target spirv -entry computeMain -stage compute -o texture-format-inference.spv
//TEST:SIMPLE: -target metal -entry computeMain -stage compute -o texture-format-inference.metal

// The texture argument must determine T directly, including in an outer generic
// function. Its operations still return the payload described by T.DataType.
T.DataType loadTexture<T : ITexelElement>(Texture2D<T> texture)
{
    return texture.mips[0][int2(0)];
}

T.DataType forwardTexture<T : ITexelElement>(Texture2D<T> texture)
{
    return loadTexture(texture);
}

T.DataType loadCombinedTexture<T : ITexelElement>(Texture2D<T> texture, SamplerState sampler)
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

void copyTexture<T : ITexelElement>(RWTexture2D<T> texture)
{
    T.DataType value = texture[int2(0)];
    texture[int2(0)] = value;
}

T.DataType loadSample<T : ITexelElement>(Texture2DMS<T> texture)
{
    return texture.sample[0][int2(0)];
}

T.DataType loadBuffer<T : ITexelElement>(Buffer<T> buffer)
{
    return buffer[0];
}

void copyBuffer<T : ITexelElement>(RWBuffer<T> buffer)
{
    T.DataType value = buffer[0];
    buffer[0] = value;
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
    formattedOutput[int2(1)] = value + singleValue.x;
}
