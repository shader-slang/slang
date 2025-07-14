//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-output-using-type -cpu

// Regression test for issue #7748: Dictionary key collision in multi-level break processing
// This test ensures that code with multi-level breaks compiles successfully and doesn't cause
// internal compiler errors due to dictionary key collisions.

int testMultiLevelBreak(int value) {
    int result = 0;
outer:
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            result++;
            if (value == 0) {
                break outer; // Multi-level break that triggers EliminateMultiLevelBreak pass
            }
            if (result > 10) {
                break outer;
            }
        }
    }
    return result;
}

int processValues() {
    int sum = 0;
    sum += testMultiLevelBreak(0);
    sum += testMultiLevelBreak(1);
    sum += testMultiLevelBreak(2);
    return sum;
}

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]  
void computeMain() {
    int result = processValues();
    outputBuffer[0] = result;
    //CHECK: 19
}