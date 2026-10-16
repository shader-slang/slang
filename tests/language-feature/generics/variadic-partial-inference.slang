//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

void aggregate(inout int v, int u)
{
    v = v * u;
}

T test<T:IInteger, each U : IInteger>(T t, expand each U u)
{
    int v = 1;
    expand aggregate(v, (each u).toInt());
    return t + T(v);
}

[numthreads(1, 1, 1)]
void computeMain()
{
    // CHECK: 11
    outputBuffer[0] = test<int>(5, 1, 2, 3);
    // Reuse the same generic with both repeated and different inferred pack lengths.
    // CHECK: 30
    outputBuffer[1] = test<int>(6, 2, 3, 4);
    // CHECK: 8
    outputBuffer[2] = test<int>(7);
    // CHECK: 28
    outputBuffer[3] = test<int>(8, 4, 5);
}
