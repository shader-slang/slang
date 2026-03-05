//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cuda -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

// Regression test: MakeValuePack is hoistable for deduplication, but must
// be inlined back into functions before emission to avoid global-scope
// dynamic initialization errors on backends like CUDA and SPIRV.

void helper(inout float r, float x) { if (x < r) r = x; }

float minVal<each T : __BuiltinFloatingPointType>(expand each T v)
{
    float result = 1e38;
    expand helper(result, __realCast<float>(each v));
    return result;
}

[numthreads(1,1,1)]
void computeMain()
{
    // Using half values triggers half-precision MakeValuePack/MakeTuple
    // that can cause global dynamic init issues on CUDA.
    outputBuffer[0] = minVal(1.0, 2.0, 0.5);
    // CHECK: 0.5

    outputBuffer[1] = minVal(10.0, 5.0);
    // CHECK: 5.0
}
