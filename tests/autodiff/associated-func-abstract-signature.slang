// A differentiable `__associatedfunc` requirement whose signature is still abstract, such as `F`
// in an interface generic over `F`, takes its differentiability info from that type.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)

interface IAbstract<F>
    where __hasDiffTypeInfo(F)
{
    [Differentiable]
    static __associatedfunc F d;
}

interface IMaybeAbstract<F>
    where __hasDiffTypeInfo(F)
{
    [MaybeDifferentiable]
    static __associatedfunc F d;
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // CHECK: {{^}}1.000000{{$}}
    outputBuffer[0] = 1.0;
}
