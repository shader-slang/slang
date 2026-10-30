//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -shaderobj -output-using-type

// Requirements an optional constraint inherits stay usable where the constraint is checked or
// where a required constraint also proves them (shader-slang/slang#13011, #13061).

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

struct NotArithmetic
{
    int x;
}

// `+` is an `IArithmetic` requirement that `IFloat` inherits.
T addIfFloat<T>(T a, T b) where optional T : IFloat
{
    if (T is IFloat)
        return a + b;
    return a;
}

U twice<U : IArithmetic>(U x)
{
    return x.add(x);
}

T twiceIfFloat<T>(T a) where optional T : IFloat
{
    if (T is IFloat)
        return twice(a);
    return a;
}

T modIfInteger<T>(T a, T b) where optional T : __BuiltinIntegerType
{
    if (T is __BuiltinIntegerType)
        return a % b;
    return a;
}

// Both constraints lead to `IArithmetic`; the required `IInteger` one proves it, whichever
// order the constraints are written in.
T addOptionalFirst<T>(T a, T b)
    where optional T : IFloat
    where T : IInteger
{
    return a + b;
}

T addRequiredFirst<T>(T a, T b)
    where T : IInteger
    where optional T : IFloat
{
    return a + b;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    NotArithmetic n = { 5 };

    // CHECK: 7
    outputBuffer[0] = int(addIfFloat<float>(3.0, 4.0));
    // CHECK-NEXT: 5
    outputBuffer[1] = addIfFloat<NotArithmetic>(n, n).x;
    // CHECK-NEXT: 2
    outputBuffer[2] = modIfInteger<int>(17, 5);
    // CHECK-NEXT: 5
    outputBuffer[3] = modIfInteger<NotArithmetic>(n, n).x;
    // CHECK-NEXT: 7
    outputBuffer[4] = addOptionalFirst<int>(3, 4);
    // CHECK-NEXT: 7
    outputBuffer[5] = addRequiredFirst<int>(3, 4);
    // CHECK-NEXT: 6
    outputBuffer[6] = int(twiceIfFloat<float>(3.0));
}
