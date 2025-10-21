//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type

interface IFoo : IDifferentiable
{
    float myFunc();
}

struct Bar: IDifferentiable
{
    float y;
}

struct Foo :  IFoo
{
    typealias Differential = Bar;

    [DerivativeMember(Differential.y)]
    float x;

    [BackwardDifferentiable]
    float myFunc()
    {
        return x * x;
    };
}

//TEST_INPUT:ubuffer(data=[0 0 0 0 0], stride=4):out,name=g_output
RWStructuredBuffer<float> g_output;

[BackwardDifferentiable]
float wrapper(Foo f)
{
    return f.myFunc();
}

[shader("compute")]
void computeMain(uint3 thread_id : SV_DispatchThreadID)
{
    var di = diffPair(Foo(1.0f), Bar(0.0f));
    bwd_diff(wrapper)(di, 1.0f);

    // CHECK: 2.0
    g_output[0] = di.d.y;
}
