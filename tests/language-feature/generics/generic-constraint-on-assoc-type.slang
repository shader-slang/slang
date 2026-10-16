// Test that a where clause constraining an associated type to a concrete type
// enables operations that depend on knowing the concrete type (e.g. arithmetic).
// Unlike where-2.slang (which returns a constant), this test uses the
// constrained associated type in arithmetic to verify the constraint is
// propagated into the function body.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -shaderobj -output-using-type

interface IProvider
{
    associatedtype DataType;
    DataType getData();
}

// The equality constraint `T.DataType == int` lets the body use int arithmetic
// on the value returned by getData(), which would not type-check without it.
int transform<T : IProvider>(T provider, int multiplier)
    where T.DataType == int
{
    return provider.getData() * multiplier + 1;
}

// The result type is inferred from the associated-type equality, without a
// value argument or explicit type argument supplying U. Both orientations of
// the equality describe the same requirement.
U inferPayload<T : IProvider, U>(T provider)
    where T.DataType == U
{
    return provider.getData();
}

U inferPayloadReversed<T : IProvider, U>(T provider)
    where U == T.DataType
{
    return provider.getData();
}

struct IntProvider : IProvider
{
    typealias DataType = int;
    int stored;
    int getData() { return stored; }
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    IntProvider p;
    p.stored = 5;

    outputBuffer[0] = transform(p, 10);
    // CHECK: 51
    outputBuffer[1] = inferPayload(p);
    // CHECK: 5
    outputBuffer[2] = inferPayloadReversed(p);
    // CHECK: 5
}
