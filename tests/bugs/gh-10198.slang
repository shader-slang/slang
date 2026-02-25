//TEST_IGNORE_FILE:

// Regression test: SPIRV structured control flow violation with switch-continue
// Issue: https://github.com/shader-slang/slang/issues/10198
// Note: gh-7748-switch-continue-bug.slang covers the CPU path. This tests SPIRV.

RWStructuredBuffer<int> outputBuffer;

int test(int value) {
    int result = 0;
    for (int i = 0; i < 3; i++) {
        switch (value) {
            case 0:
                result += 1;
                continue;
            case 1:
                result += 2;
                break;
            default:
                result += 3;
                break;
        }
        result += 10;
    }
    return result;
}

[shader("compute")]
[numthreads(1,1,1)]
void computeMain() {
    outputBuffer[0] = test(0) + test(1) + test(2);
}
