//TEST:SIMPLE(filecheck=CHECK): -target hlsl -stage compute -entry computeMain

// Regression for shader-slang/slang#12762: a typed/texel `Buffer<T>` must conform to
// `IArray<T>`, and `RWBuffer<T>` / `RasterizerOrderedBuffer<T>` to `IRWArray<T>`, so that
// generic code constrained on those interfaces accepts a texel buffer argument (as it
// already does for `StructuredBuffer`). Before the fix, `first(typedBuffer)` failed with
// "could not specialize generic for arguments of type (Buffer<float>)".

T first<T, C : IArray<T>>(C values)
{
    return values[0];
}

void writeFirst<U, C : IRWArray<U>>(C values, U value)
{
    values[0] = value;
}

Buffer<float> typedBuffer;
RWBuffer<float> rwTypedBuffer;
RasterizerOrderedBuffer<float> rasterBuffer;

//TEST_INPUT:ubuffer(data=[0 0], stride=8):out,name=output
RWStructuredBuffer<float2> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    //CHECK: void computeMain
    writeFirst(rwTypedBuffer, first(typedBuffer));
    output[0] = float2(first(typedBuffer), rwTypedBuffer[0]) + float2(first(rasterBuffer), 0);
}
