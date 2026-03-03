// Variadic generic interfaces (IFoo<each Ts>) with dynamic dispatch.
// Tests whether an interface parameterized by a variadic type pack
// can be used as a dynamically dispatched type.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=REPORT):-target hlsl -stage compute -entry computeMain -report-dynamic-dispatch-sites

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

interface IAccumulator<each Ts>
{
    float accumulate(expand each Ts values);
}

struct SumTwo : IAccumulator<float, float>
{
    float accumulate(float a, float b) { return a + b; }
}

struct ProductTwo : IAccumulator<float, float>
{
    float accumulate(float a, float b) { return a * b; }
}

IAccumulator<float, float> makeAccum(int id)
{
    if (id == 0) return SumTwo();
    else return ProductTwo();
}

float test(int id, float a, float b)
{
    let acc = makeAccum(id);
    return acc.accumulate(a, b);
    // REPORT: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
}

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = test(0, 3.0, 4.0); // CHECK: 7.0
    outputBuffer[1] = test(1, 3.0, 4.0); // CHECK: 12.0
}
