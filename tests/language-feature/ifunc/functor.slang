//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -use-dxil -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -shaderobj -output-using-type

struct Functor : IMutatingFunc<float, float>
{
    int context;

    [mutating]
    float __call(float p)
    {
        context += (int)p;
        return context;
    }
}

float apply<T:IMutatingFunc<float,float>>(inout T f, float p)
{
    return f(p);
}

//TEST_INPUT:ubuffer(data=[0 3 2 2], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint tid: SV_DispatchThreadID)
{
    Functor f;
    f.context = 0;

    f(1.0f);
    apply(f, 2.0f);
    apply(f, 3.0f);

    // CHECK: 6
    outputBuffer[0] = (uint)f.context;
}
