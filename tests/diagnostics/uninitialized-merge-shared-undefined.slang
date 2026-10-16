//DIAGNOSTIC_TEST:SIMPLE(diag=CHK): -target spirv -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHK): -target spirv -entry computeMain -stage compute -g

// Code generation forwards a block param whose incoming values are all the same undefined value,
// as happens for the loop below once its inner merge is folded. The front end's uninitialized-use
// check runs before that, so it still sees each never-written variable on its own, and the warnings
// are the same with and without debug info.

cbuffer C
{
    int n;
    bool b;
};

RWStructuredBuffer<float> outputBuffer;

float loopWithUndefinedReassign()
{
    float t;
    for (int i = 0; i < n; i++)
    {
        if (b)
        {
            float r;
            t = r;
        }
    }
    return t;
}

float mergeOfSameUndefined()
{
    float t;
    if (b)
        outputBuffer[1] = 2.0;
    return t;
/*CHK:
    ^^^^^^ use of uninitialized variable
    ^^^^^^ use of uninitialized variable 't'
*/
}

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = loopWithUndefinedReassign() + mergeOfSameUndefined();
}
