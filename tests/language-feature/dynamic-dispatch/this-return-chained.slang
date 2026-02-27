// When an interface method returns `This`, each call in a chain must go
// through dynamic dispatch to select the correct implementation. This test
// exercises multi-step chaining (obj.f().f().g()), storing intermediate
// interface-typed results, and passing them to functions that chain further.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-dx12 -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-mtl -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=REPORT):-target hlsl -stage compute -entry computeMain -report-dynamic-dispatch-sites

interface ITransformable
{
    This transform(float factor);
    float getValue();
}

struct Adder : ITransformable
{
    float v;
    This transform(float factor) { return Adder(v + factor); }
    float getValue() { return v; }
}

struct Multiplier : ITransformable
{
    float v;
    This transform(float factor) { return Multiplier(v * factor); }
    float getValue() { return v; }
}

float applyTwice(ITransformable obj, float factor)
{
    return obj.transform(factor).transform(factor).getValue();
    // REPORT: ([[# @LINE-1]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
    // REPORT: ([[# @LINE-2]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
    // REPORT: ([[# @LINE-3]]): {{.*}} generated {{.*}} dispatch code{{.*}} 2 possible types:
}

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    ITransformable obj;
    if (outputBuffer[0] == 0.0)
        obj = Adder(1.0);
    else
        obj = Multiplier(1.0);

    // Multi-step chain: Adder(1).transform(10) -> Adder(11).transform(5) -> Adder(16)
    outputBuffer[0] = obj.transform(10.0).transform(5.0).getValue();
    // CHECK: 16.0

    // Store intermediate result as interface-typed value (This identity erased via dispatch)
    ITransformable mid = obj.transform(100.0);
    outputBuffer[1] = mid.getValue();
    // CHECK: 101.0

    // Pass stored result to function that chains further
    // applyTwice(Adder(101), 3.0) -> Adder(101+3+3) = Adder(107)
    outputBuffer[2] = applyTwice(mid, 3.0);
    // CHECK: 107.0

    // Same patterns with a statically-known Multiplier to exercise the
    // specialization path. Because the concrete type is known at assignment,
    // the compiler specializes away dynamic dispatch for the mul.* call sites.
    // The REPORT patterns above are in applyTwice, which is reached from the
    // interface-typed obj path only.
    ITransformable mul = Multiplier(2.0);
    outputBuffer[3] = mul.transform(3.0).transform(4.0).getValue();
    // CHECK: 24.0

    outputBuffer[4] = applyTwice(mul, 5.0);
    // CHECK: 50.0
}
