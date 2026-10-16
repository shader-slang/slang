// SlangPy wraps texture arguments in generic helper structs/methods that
// operate on `Texture2D<T>` with `T : ITexelElement`. Explicit texture format
// descriptors changed public texture aliases to route through `T.DataType`;
// this test keeps the ordinary generic `float4` path type-checking.

//TEST:SIMPLE: -target spirv -entry computeMain -stage compute -o slangpy-generic-texture-element.spv
//TEST:SIMPLE: -target hlsl -entry computeMain -stage compute -o slangpy-generic-texture-element.hlsl

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

// SlangPy exposes one-channel textures as scalars. The explicit conformance
// constraint must retain the identity DataType binding for the vector, so the
// original swizzle and scalar store remain valid without a conversion temporary.
public extension<T> RWTexture2DType<vector<T, 1>>
    where vector<T, 1> : ITexelElement
{
    public void __slangpy_load(Context2D context, out T result)
    {
        result = this.value[context.callID].x;
    }

    public void __slangpy_store(Context2D context, T input)
    {
        this.value[context.callID] = input;
    }
}

public struct Context3D
{
    uint3 call_id_vec;
}

public struct RWTexture3DType<T : ITexelElement>
{
    RWTexture3D<T> value;
}

public extension<T> RWTexture3DType<vector<T, 1>>
    where vector<T, 1> : ITexelElement
{
    public void __slangpy_load(Context3D context, out T value)
    {
        value = this.value[context.call_id_vec].x;
    }

    public void __slangpy_store(Context3D context, in T value)
    {
        this.value[context.call_id_vec] = value;
    }
}

Texture2D<float4> inputTexture;
RWTexture2D<float4> outputTexture;
RWTexture2D<float1> scalarTexture2D;
RWTexture3D<float1> scalarTexture3D;

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

    RWTexture2DType<float1> scalarWrapper2D;
    scalarWrapper2D.value = scalarTexture2D;
    float scalar;
    scalarWrapper2D.__slangpy_load(context, scalar);
    scalarWrapper2D.__slangpy_store(context, scalar);

    Context3D context3D;
    context3D.call_id_vec = dispatchThreadID;
    RWTexture3DType<float1> scalarWrapper3D;
    scalarWrapper3D.value = scalarTexture3D;
    scalarWrapper3D.__slangpy_load(context3D, scalar);
    scalarWrapper3D.__slangpy_store(context3D, scalar);
}
