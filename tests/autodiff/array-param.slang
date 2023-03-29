
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

struct MyType
{
    static const int kSize = 2;
}
static const int kSize0 = 4;

struct Obj
{
    void readArr(out float arr[6])
    {
        [ForceUnroll]
        for (int i = 0; i < 6; i++)
            arr[i] = i;
    }

    [BackwardDerivativeOf(readArr)]
    void readArr_bwd(float arr_d[6])
    {
        outputBuffer[0] = arr_d[0];
    }

    float member;
    [BackwardDifferentiable]
    void f(float arr[kSize0 + MyType.kSize], out float result[3])
    {
        float sum = 0;
        [ForceUnroll]
        for (int i = 0; i < kSize0 + MyType.kSize; i++)
            sum += arr[i];
        result[0] = sum;
        result[1] = sum;
        result[2] = sum;
    }
}

[BackwardDifferentiable]
float g()
{
    float arr[kSize0 + MyType.kSize];
    Obj o;
    o.member = 1;
    o.readArr(arr);
    float res[3];
    o.f(arr, res);
    return res[0] + res[1] + res[2];
}


[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    __bwd_diff(g)(1.0);
}
