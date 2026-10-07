// A declaration and a definition whose `__ref` parameters differ only in `readonly` declare the
// same function, so the call resolves to the one definition instead of becoming ambiguous.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

int read(readonly __ref int x);
int read(__ref int x) { return x + 1; }

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    int a = 41;
    // CHECK: 42
    outputBuffer[0] = read(a);
}
