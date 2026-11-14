//DIAGNOSTIC_TEST:SIMPLE(diag=CHK):

// Test if the variable name is a part of the error message
// when it is used inside of for-loop

RWStructuredBuffer<float> gInput;
RWStructuredBuffer<float> outputBuffer;

float func1() { float a; return a; }
//CHK:                   ^^^^^^ use of uninitialized variable
//CHK:                   ^^^^^^ use of uninitialized variable 'a'

float func2() { float b; return b; }
//CHK:                   ^^^^^^ use of uninitialized variable
//CHK:                   ^^^^^^ use of uninitialized variable 'b'

int test(int inVal)
{
    return inVal;
}

[Shader("compute")]
[NumThreads(4, 1, 1)]
void computeMain(int3 dispatchThreadID : SV_DispatchThreadID)
{
    int tid = dispatchThreadID.x;
    int inVal1; // intentionally uninitialized
    int inVal2; // intentionally uninitialized

    for (int i = 0; i <2; ++i)
    {
        int outVal = test(inVal1);
//CHK:                   ^ use of uninitialized variable
//CHK:                   ^ use of uninitialized variable 'inVal1'

        outVal += test(inVal2);
//CHK:                ^ use of uninitialized variable
//CHK:                ^ use of uninitialized variable 'inVal2'
        outputBuffer[tid] = outVal;
    }
}
