// SlangPy wraps texture arguments in generic helper structs/methods that
// operate on `Texture2D<T>` with `T : ITexelElement`. Explicit texture format
// descriptors changed public texture aliases to route through `T.DataType`;
// this test keeps the ordinary generic `float4` path type-checking.

//TEST:SIMPLE: -target spirv -entry computeMain -stage compute -o slangpy-generic-texture-element.spv

public struct Context2D
{
    int2 callID;
}

public struct Texture2DType<T : ITexelElement>
{
    Texture2D<T> value;

    public void __slangpy_load(Context2D context, out T result)
    {
        result = value[context.callID];
    }
}

public struct RWTexture2DType<T : ITexelElement>
{
    RWTexture2D<T> value;

    public void __slangpy_store(Context2D context, T input)
    {
        value[context.callID] = input;
    }
}

Texture2D<float4> inputTexture;
RWTexture2D<float4> outputTexture;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    Context2D context;
    context.callID = int2(dispatchThreadID.xy);

    Texture2DType<float4> inputWrapper;
    inputWrapper.value = inputTexture;

    RWTexture2DType<float4> outputWrapper;
    outputWrapper.value = outputTexture;

    float4 value;
    inputWrapper.__slangpy_load(context, value);
    outputWrapper.__slangpy_store(context, value);
}
