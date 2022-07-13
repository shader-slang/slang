//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

import test_intrinsics;

float my_pow_jvp(float x, float n, float dx, float dn)
{
    return dx * n * pow(x, n-1) + dn * pow(x, n) * log(x);
}

[__custom_jvp(my_pow_jvp)]
float _pow(float, float);

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
     {
        float a = 5.0;
        float n = 2;
        float da = 1.0;
        float dn = 0;

        outputBuffer[0] = __jvp(_pow)(a, n, da, dn);        // Expect: 10.0
        outputBuffer[1] = __jvp(_pow)(a, n, 0.0, 1.0);      // Expect: 40.23595
    }
}
