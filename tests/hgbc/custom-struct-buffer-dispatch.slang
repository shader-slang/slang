// NOT A TEST (don't make it one)

public struct ElementType
{
    public float3 a;
    public uint b;
}

public struct BufferPair
{
    public RWStructuredBuffer<ElementType> first;
    public RWStructuredBuffer<ElementType> second;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void writePair(uniform BufferPair buffers)
{
    buffers.first[0] = ElementType(
        float3(33, 33, 33), 33);
}

[host]
public void main(BufferPair buffers)
{
    dispatch(writePair, uint3(1, 1, 1))(buffers);
}


// Example implementation of a radix sort module.

module radix_sort;

interface ISortKey
{
    uint8_t getByte(uint8_t i);
}

[shader("compute")]
void sortPassCS<TKey : ISortKey, TVal>(
    RWStructuredBuffer<TKey> keyBuffer,
    RWStructuredBuffer<TVal> valueBuffer,
    RWStructuredBuffer<TKey> scratchKeyBuffer,
    RWStructuredBuffer<TVal> scratchValueBuffer)
{
    /* ... */
}

struct RadixSort<TKey : ISortKey, TVal>
{
    RWStructuredBuffer<TKey> scratchKeyBuffer;
    RWStructuredBuffer<TVal> scratchValueBuffer;

    void sortInPlace(RWStructuredBuffer<TKey> keyBuffer, RWStructuredBuffer<TVal> valueBuffer)
    {
        /* ... */
        for (/*...*/)
        {
            dispatch(sortPassCS<TKey, TVal>, groupCount)(
                keyBuffer, valueBuffer,
                scratchKeyBuffer, scratchValueBuffer);
            swap(keyBuffer, scratchKeyBuffer);
            swap(valueBuffer, scratchValueBuffer);
            // ...
        }
    }
}



// Example reusable library implementation of a buffer map using a lambda.
module buffer_map;

void mapPassCS<T, U, TFunc : IFunc<U, T>>(
    RWStructuredBuffer<T> inputBuffer,
    RWStructuredBuffer<U> outputBuffer,
    uniform TFunc func)
{
    // do thread ID guard ...
    outputBuffer[i] = func(inputBuffer[i]);
}

extension<T> RWStructuredBuffer<T>
{
    RWStructuredBuffer<U> map<U, TFunc : IFunc<U, T>>(uniform TFunc func)
    {
        var output = RWStructuredBuffer<U>::alloc(this.getCount());
        dispatch(mapPassCS<T, U, TFunc>, uint3(this.getCount(), 1, 1))(
            this, output, func);
        return output;
    }
}

// User code.
import buffer_map;

[host]
RWStructuredBuffer<float> saxpy(RWStructuredBuffer<float> x, float a, float b)
{
    // a and b are captured into the lambda's state struct
    // and passed how a struct usually is (no special logic needed)
    //
    return x.map<float>((float v) => a * v + b);
}


// Example reusable library implementation of a buffer map using a lambda.
module buffer_map;

void mapPassCS<T, U, TFunc : IFunc<U, T>>(
    RWStructuredBuffer<T> inputBuffer,
    RWStructuredBuffer<U> outputBuffer,
    uniform TFunc func)
{
    // do thread ID guard ...
    outputBuffer[i] = func(inputBuffer[i]);
}

extension<T> RWStructuredBuffer<T>
{
    RWStructuredBuffer<U> map<U, TFunc : IFunc<U, T>>(uniform TFunc func)
    {
        var output = RWStructuredBuffer<U>::alloc(this.getCount());
        dispatch(mapPassCS<T, U, TFunc>, uint3(this.getCount(), 1, 1))(
            this, output, func);
        return output;
    }
}

// User code.
import buffer_map;

[host]
[Differentiable]
DiffTensorView<float> saxpy(DiffTensorView<float> x, float a, float b)
{
    // Experimental: lambdas in differentiable host code are
    // captured as IDifferentiableFunc if possible,
    //
    // "map" has a custom derivative that calls another shader to
    // reduce the gradients for the uniforms.
    //
    return x.map<float>((float v) => a * v + b);
}
