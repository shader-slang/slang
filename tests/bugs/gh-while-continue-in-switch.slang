//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-output-using-type -cpu

// Test for continue inside switch inside while loop
// This is a regression test for a compiler exception caused by dictionary key collision
// when processing multi-level breaks/continues.

int testContinueInSwitchInWhile(int maxIter, int continueValue) {
    int result = 0;
    int iter = 0;
    
    while (iter < maxIter) {
        switch (iter) {
        case 0:
            result += 1;
            if (iter == continueValue) {
                iter++;
                continue; // Continue to while loop
            }
            break;
        case 1:
            result += 2;
            if (iter == continueValue) {
                iter++;
                continue; // Continue to while loop
            }
            break;
        case 2:
            result += 3;
            if (iter == continueValue) {
                iter++;
                continue; // Continue to while loop
            }
            break;
        default:
            break;
        }
        
        result += 10;
        iter++;
    }
    
    return result;
}

int processValues() {
    int sum = 0;
    // Test with continue at case 0: result = 1, then (2+10), then (3+10) = 26
    sum += testContinueInSwitchInWhile(3, 0);
    // Test with continue at case 1: result = (1+10), 2, then (3+10) = 26
    sum += testContinueInSwitchInWhile(3, 1);
    // Test with no continues: result = (1+10), (2+10), (3+10) = 36
    sum += testContinueInSwitchInWhile(3, 999);
    return sum;
}

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain() {
    int result = processValues();
    outputBuffer[0] = result;
    //CHECK: 88
}
