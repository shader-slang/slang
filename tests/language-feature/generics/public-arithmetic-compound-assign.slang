// Compound-assignment operators (`+= -= *= /= %=`) are available for generic code constrained to
// the public arithmetic interfaces (`IArithmetic` / `IFloat`), including when the type parameter
// is itself a vector, consistent with the logical family (`&= |= ^=` over `ILogical`).

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

T accumulate<T : IArithmetic>(T a, T b)
{
    T x = a;
    x += b;
    x -= b;
    x *= b;
    return x;
}

T remainderInto<T : IArithmetic>(T a, T b)
{
    T x = a;
    x %= b;
    return x;
}

T scaleUp<T : IFloat>(T a, T b)
{
    T x = a;
    x /= b;
    return x;
}

T orInto<T : ILogical>(T a, T b)
{
    T x = a;
    x |= b;
    return x;
}

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    int ai = accumulate(5, 2);
    //CHECK: 10
    outputBuffer[0] = float(ai);

    float af = accumulate(5.0f, 2.0f);
    //CHECK: 10.0
    outputBuffer[1] = af;

    // The type parameter can itself be a vector: accumulate<float2> exercises float2 += float2 etc.
    // (5,4) += (2,1) -> (7,5); -= (2,1) -> (5,4); *= (2,1) -> (10,4); sum = 14.
    float2 av = accumulate(float2(5.0f, 4.0f), float2(2.0f, 1.0f));
    //CHECK: 14.0
    outputBuffer[2] = av.x + av.y;

    int r = remainderInto(7, 3);
    //CHECK: 1
    outputBuffer[3] = float(r);

    float s = scaleUp(8.0f, 2.0f);
    //CHECK: 4.0
    outputBuffer[4] = s;

    int o = orInto(5, 2);
    //CHECK: 7
    outputBuffer[5] = float(o);
}
