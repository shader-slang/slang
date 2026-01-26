
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

struct Composite { int member; }
TResult helper<TResult, TFunc:IFunc<TResult, int>>(TFunc f)
{
    return f(0);
}

[numthreads(1,1,1)]
void computeMain()
{
    let c = helper( (int x) => Composite(3) );
    outputBuffer[0] = c.member;
    // CHECK: 3
}