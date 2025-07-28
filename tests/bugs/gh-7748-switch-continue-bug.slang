//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-output-using-type -cpu

// Regression test for issue #7748: Dictionary key collision in multi-level break processing
// This test specifically exercises the case of "continue inside a switch that is inside a for loop"
// which was identified as the root cause of the dictionary collision issue.

int testContinueInSwitchInLoop(int value) {
    int result = 0;

    for (int i = 0; i < 3; ++i) {
        switch (value) {
            case 0:
                result += 1;
                continue; // This continue should go to the for loop
            case 1:
                result += 2;
                break; // This break goes to the switch
            default:
                result += 3;
                break;
        }
        result += 10; // This should be skipped when continue is used
    }

    return result;
}

int processValues() {
    int sum = 0;
    sum += testContinueInSwitchInLoop(0); // Should be 3 (1+1+1, no +10s due to continue)
    sum += testContinueInSwitchInLoop(1); // Should be 36 (2+10, 2+10, 2+10)  
    sum += testContinueInSwitchInLoop(2); // Should be 39 (3+10, 3+10, 3+10)
    return sum;
}

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]  
void computeMain() {
    int result = processValues();
    outputBuffer[0] = result;
    //CHECK: 78
}