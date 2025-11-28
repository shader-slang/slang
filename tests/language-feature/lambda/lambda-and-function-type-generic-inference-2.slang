//TEST:SIMPLE(filecheck=CHECK): -target spirv -stage compute -entry computeMain

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

func foo<let N : int>(f: functype()->vector<float, N>)->vector<float, N> { return f(); }

struct Functor : IFunc<Array<int,2>>
{
    Array<int,2> operator()()
    {
        return Array<int,2>();
    }
}

[numthreads(1,1,1)]
func computeMain()->void
{
    //CHECK: ([[# @LINE+1]]): error 39999
    outputBuffer[0] = (int)foo(() => Array<int,2>());

    //CHECK: ([[# @LINE+2]]): error 39999
    Functor f;
    outputBuffer[2] = (int)foo(f);
}