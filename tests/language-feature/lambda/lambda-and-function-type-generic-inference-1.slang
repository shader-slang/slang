//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

// This test ensures that lambdas correctly unify-types with function-types so that
// generic-parameters can be infered.
func foo<let N : int>(f: functype()->vector<float, N>)->vector<float, N> { return f(); }

struct Functor : IFunc<vector<float, 1>>
{
    vector<float, 1> operator()()
    {
        return 4.0;
    }
}

func foo3<let N:int>(f: IFunc<vector<float, N>>) {}

[numthreads(1,1,1)]
func computeMain()->void
{
    //CHECK: 2
    outputBuffer[0] = (int)foo(() => 2);
    //CHECK: 3
    outputBuffer[1] = (int)foo(() => vector<float, 1>(3));
    //CHECK: 4
    Functor f;
    outputBuffer[2] = (int)foo(f);
}