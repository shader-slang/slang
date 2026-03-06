//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cuda -shaderobj -output-using-type -xslang -g2
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

void helper(inout float r, float x) { if (x < r) r = x; }

[ForceInline]
float minVal<each T : __BuiltinFloatingPointType>(expand each T v)
{
    float result = 1e38;
    expand helper(result, __realCast<float>(each v));
    return result;
}

[ForceInline]
float minValWrapper<each T:__BuiltinFloatingPointType>(expand each T v)
{
    return minVal(v);
}

[numthreads(1,1,1)]
void computeMain()
{
    let tuple = makeTuple(1.0, 2.0, 0.5);

    // expand tuple is allowed as argument to a generic type pack parameter.
    outputBuffer[0] = minVal(expand each tuple);
    // CHECK: 0.5

    outputBuffer[1] = minVal(10.0, 5.0);
    // CHECK: 5.0
}
