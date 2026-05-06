//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

[anyValueSize(16)]
public interface IFoo
{
    [Differentiable]
    uint read();
}

//TEST_INPUT: type_conformance Foo:IFoo = 1;
//TEST_INPUT: set fPtr = ubuffer(data=[0 0 1 0 0 0 0 0], stride=4)
uniform IFoo* fPtr;

public struct Foo : IFoo
{
    [Differentiable]
    float read()
    {
        return 123.0;
    }
}

[Differentiable]
float test()
{
    return fPtr->read();
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    let dp = fwd_diff(test)();
    outputBuffer[0] = dp.p; // CHECK: 123.0
}
