// One-argument `createDynamicObject<T>(typeId)` works for non-generic
// stateless interfaces (conforming types have no data fields).
// Counterpart to generic-interface-8.slang which covers generic interfaces.
//
// See #10063 for the original "could not specialize generic" failure.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type

interface IShape
{
    float area();
}

struct Circle : IShape
{
    float area() { return 3.0; }
}

struct Square : IShape
{
    float area() { return 5.0; }
}

//TEST_INPUT: type_conformance Circle:IShape = 0
//TEST_INPUT: type_conformance Square:IShape = 1

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

IShape makeShape(uint id)
{
    return createDynamicObject<IShape>(id);
}

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = makeShape(0).area();
    // CHECK: 3.0
    outputBuffer[1] = makeShape(1).area();
    // CHECK-NEXT: 5.0
    // The existing two-argument form continues to work; regression check.
    let circleExplicit = createDynamicObject<IShape, uint>(0, 0);
    outputBuffer[2] = circleExplicit.area();
    // CHECK-NEXT: 3.0
}
