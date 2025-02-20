//TEST:SIMPLE(filecheck=CHK):

// Test if the variable name is a part of the error message
// when it is used inside of for-loop

RWStructuredBuffer<float> gInput;
RWStructuredBuffer<float> outputBuffer;

int test(int inVal)
{
    return inVal;
}

[Shader("compute")]
[NumThreads(4, 1, 1)]
void computeMain(int3 dispatchThreadID : SV_DispatchThreadID)
{
    int tid = dispatchThreadID.x;
    int inVal; // = tid; // intentionally uninitialized

    for (int i = 0; i <2; ++i)
    {
        // CHK: ([[#@LINE+1]]): warning 41016: use of uninitialized variable 'inVal'
        int outVal = test(inVal);
        outputBuffer[tid] = outVal;
    }
}
