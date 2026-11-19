// Generic argument inference prefers the type of an `inout` argument when it
// joins the types of several arguments for one type parameter.
//
// Consider `f<T>(T b, inout T a)` called with a `uint` value and an `int`
// variable. Joining `uint` and `int` on their own would pick `uint`, but
// picking `uint` here would make the `inout` argument pay for a temporary
// that is converted back after the call. The solver therefore records that
// the `int` came from an argument whose storage the parameter binds, and
// the join picks `T = int`.
//
// The preference also survives later joins. For `g<T>(inout T a, T b, T c)`
// called with an `int` variable and two `uint` values, the first join gives
// `int`, and the joined type keeps the record of the `inout` argument, so the
// join with the second `uint` value still picks `int`.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

T f<T : IArithmetic>(T b, inout T a)
{
    return b + a;
}

T g<T : IArithmetic>(inout T a, T b, T c)
{
    return a + b + c;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    uint b = 1;
    int a = -2;

    // `T = int`, so the sum is `-1` rather than a wrapped-around `uint`.
    // CHECK: -1
    outputBuffer[0] = float(f(b, a));

    // Swapping which argument is `inout` flips the preference to `uint`.
    uint c = 1;
    int d = -2;
    // CHECK-NEXT: 4294967296
    outputBuffer[1] = float(f(d, c));

    // `T = int` again, so the sum is `-1`.
    int e = -3;
    uint f1 = 1;
    uint f2 = 1;
    // CHECK-NEXT: -1
    outputBuffer[2] = float(g(e, f1, f2));
}
