// Generic interfaces with multiple type parameters should support
// dynamic dispatch. Tests IConverter<T, U> where T and U are different
// types, verifying that multi-parameter generic interfaces dispatch correctly.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=REPORT):-target hlsl -stage compute -entry computeMain -report-dynamic-dispatch-sites

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

interface IConverter<T, U>
{
    U convert(T input);
}

struct IntToFloatScale : IConverter<int, float>
{
    float convert(int input) { return float(input) * 2.5; }
}

struct IntToFloatNegate : IConverter<int, float>
{
    float convert(int input) { return float(-input); }
}

IConverter<int, float> makeConverter(int id)
{
    if (id == 0) return IntToFloatScale();
    else return IntToFloatNegate();
}

float test(int id, int input)
{
    let conv = makeConverter(id);
    return conv.convert(input);
    // REPORT: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
}

[numthreads(1, 1, 1)]
void computeMain()
{
    // IntToFloatScale: 4 * 2.5 = 10.0
    outputBuffer[0] = test(0, 4); // CHECK: 10.0
    // IntToFloatNegate: -4 = -4.0
    outputBuffer[1] = test(1, 4); // CHECK: -4.0
}
