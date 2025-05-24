//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

struct X<B, each T, each U>
{
    int getTSize() { return countof(T); }
    int getUSize() { return countof(U); }
}

func foo<each T, each U>() -> X<bool, expand Ptr<each T>, int, expand Ptr<each U>, float> // unify
{
    return {};
}

[numthreads(1,1,1)]
void computeMain()
{
    let x = foo<int, float>();

    outputBuffer[0] = x.getTSize();
    outputBuffer[1] = x.getUSize();
    // CHECK-COUNT-2: 2
}
