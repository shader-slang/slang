// `countof(array)` is resolved by target-shared AST->IR lowering and IR
// specialization, so it must fold to the element count on every target, not
// just the interpreter; this exercises the CPU backend. (shader-slang/slang#11317)

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    int a5[5];
    double d5[5];
    uint16_t h5[5];
    int2 v3[3];

    outputBuffer[0] = countof(a5);
    outputBuffer[1] = countof(d5);
    outputBuffer[2] = countof(h5);
    outputBuffer[3] = countof(v3);

    // CHECK: 5
    // CHECK: 5
    // CHECK: 5
    // CHECK: 3
}
