//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -shaderobj -output-using-type

// A typealias in an interface is a plain alias for the interface's own signatures,
// not a requirement: conforming types can use it without declaring it.

interface IFoo<T : __BuiltinFloatingPointType>
{
    typealias V = vector<T, 3>;
    T sum(V v);
}

struct Bar<T : __BuiltinFloatingPointType> : IFoo<T>
{
    T sum(V v) { return v.x + v.y + v.z; }
}

//TEST_INPUT: set resultBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<float> resultBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    IFoo<float> foo = Bar<float>();
    IFoo<float>.V v = float3(1, 2, 3);
    // CHECK: 6.0
    resultBuffer[0] = foo.sum(v);
}
