//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -shaderobj -output-using-type -cpu

// Regression test for the `_coerce` fast-rejection of opaque generic type
// parameters into scalar builtins (overload-resolution perf optimization).
//
// `poly` exercises the *short-circuited* path: ranking `a*a + b*a + ...` on a
// constrained generic `T` rejects the many concrete builtin scalar operator
// overloads (`operator*(float,float)` etc.) by failing to coerce `T` -> scalar.
// `broadcast` exercises the *preserved* path: a scalar generic `T` still
// broadcasts into `vector<T,3>` via its element constructor (toType is a vector,
// not a scalar, so the fast-rejection must not fire). Both must still type-check
// and produce correct values.

T poly<T : IArithmetic>(T a, T b)
{
    return a * a + b * a + a + b;
}

vector<T, 3> broadcast<T : IArithmetic>(T x)
{
    return vector<T, 3>(x); // scalar generic T -> vector<T,3> broadcast
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = poly<int>(3, 4);
    // CHECK: 28

    outputBuffer[1] = poly<int>(2, 5);
    // CHECK: 21

    outputBuffer[2] = int(poly<uint>(3u, 4u));
    // CHECK: 28

    int3 v = broadcast<int>(7);
    outputBuffer[3] = v.x;
    // CHECK: 7
    outputBuffer[4] = v.y + v.z;
    // CHECK: 14

    outputBuffer[5] = poly<int>(0, 6);
    // CHECK: 6
}
