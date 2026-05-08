//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

typedef DifferentialPair<float> dpfloat;

interface IValue
{
    property no_diff float value { get; }
}

struct ValueImpl : IValue
{
    no_diff float value;
}

interface IValueRW
{
    property no_diff float value { get; set; }
}

struct ValueImplRW : IValueRW
{
    no_diff float value;
}

[ForwardDifferentiable]
float directTest(ValueImpl obj, float x)
{
    return obj.value * x;
}

[ForwardDifferentiable]
float genericTest<T : IValue>(T obj, float x)
{
    return obj.value * x;
}

[ForwardDifferentiable]
float directSetTest(ValueImplRW obj, ValueImpl src, float y)
{
    obj.value = src.value;
    return obj.value * y;
}

[ForwardDifferentiable]
float genericSetTest<T : IValueRW, U : IValue>(T obj, U src, float y)
{
    obj.value = src.value;
    return obj.value * y;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    ValueImpl obj;
    obj.value = 3.0;

    let direct = __fwd_diff(directTest)(obj, dpfloat(2.0, 1.0));
    outputBuffer[0] = direct.p;
    outputBuffer[1] = direct.d;

    let generic = __fwd_diff(genericTest)(obj, dpfloat(2.0, 1.0));
    outputBuffer[2] = generic.p;
    outputBuffer[3] = generic.d;

    ValueImplRW obj2;
    obj2.value = 1.0;

    let directSet = __fwd_diff(directSetTest)(obj2, obj, dpfloat(2.0, 1.0));
    outputBuffer[4] = directSet.p;
    outputBuffer[5] = directSet.d;

    let genericSet = __fwd_diff(genericSetTest)(obj2, obj, dpfloat(2.0, 1.0));
    outputBuffer[6] = genericSet.p;
    outputBuffer[7] = genericSet.d;

    // CHECK: 6.0
    // CHECK: 3.0
    // CHECK: 6.0
    // CHECK: 3.0
    // CHECK: 6.0
    // CHECK: 3.0
    // CHECK: 6.0
    // CHECK: 3.0
}
