//TEST:SIMPLE(filecheck=CHECK): -entry compute_main -output-using-type

// Regression test for issue #7748: Dictionary key collision in loop processing
// This test ensures that for-loops with dynamic bounds can be compiled without
// internal dictionary key collision errors.

struct TestData {
    int primitives[10];
};

void processLoop(int start, int end, out int result) {
    result = 0;
    for (int i = start; i < end; ++i) {
        result += i;
        // nested control flow to test break/continue labels
        if (i == 3) {
            continue;
        }
        if (i > 7) {
            break;
        }
    }
}

int testScene(int value) {
    int result1, result2;
    processLoop(0, 5, result1);
    processLoop(2, 8, result2); 
    return result1 + result2;
}

[shader("compute")]
[numthreads(1, 1, 1)]  
void compute_main() {
    int result = testScene(42);
    //CHECK: result: 25
    printf("result: %d\n", result);
}