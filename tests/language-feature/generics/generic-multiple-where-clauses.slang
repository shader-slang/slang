// Test that a generic function can use multiple where clauses combining
// conformance and associated type equality constraints on separate type params.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -shaderobj -output-using-type

interface IContainer
{
    associatedtype ElementType;
    ElementType getElement();
}

interface IScalable
{
    int getScale();
}

// Multiple where clauses: two conformance constraints + equality on associated type
int scaledElement<T, U>(T container, U scaler)
    where T : IContainer
    where U : IScalable
    where T.ElementType == int
{
    return container.getElement() * scaler.getScale();
}

struct IntBox : IContainer
{
    typealias ElementType = int;
    int val;
    int getElement() { return val; }
}

struct Multiplier : IScalable
{
    int factor;
    int getScale() { return factor; }
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    IntBox box;
    box.val = 7;

    Multiplier mul;
    mul.factor = 3;

    outputBuffer[0] = scaledElement(box, mul);
    // CHECK: 21
}
