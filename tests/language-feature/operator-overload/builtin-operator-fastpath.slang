// Regression/correctness test for the hard-coded builtin-operator fast path
// (convertToBuiltinArithmeticOp / lowerBuiltinArithmeticOp). All operators below act on
// runtime values (read from the buffer) so they are genuinely fast-pathed (not folded),
// exercising arithmetic, comparison, bitwise, equality, and unary operators on scalar and
// vector operands.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-d3d12 -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // Runtime operands (value-dependent so nothing constant-folds away).
    int a = outputBuffer[0] + 7; // 7
    int b = outputBuffer[1] + 3; // 3

    outputBuffer[0] = a + b;          // 10
    outputBuffer[1] = a - b;          // 4
    outputBuffer[2] = a * b;          // 21
    outputBuffer[3] = a / b;          // 2
    outputBuffer[4] = a % b;          // 1
    outputBuffer[5] = (a > b) ? 1 : 0;  // 1
    outputBuffer[6] = (a == b) ? 1 : 0; // 0
    outputBuffer[7] = (a & b);        // 3
    outputBuffer[8] = (a | b);        // 7
    outputBuffer[9] = (a ^ b);        // 4
    outputBuffer[10] = (a << b);      // 56
    outputBuffer[11] = (a >> b);      // 0
    outputBuffer[12] = -a;            // -7
    outputBuffer[13] = ~a;            // -8

    // Vector operators.
    int4 va = int4(a, b, a + b, a - b); // (7,3,10,4)
    int4 vb = va * 2 - 1;               // (13,5,19,7)
    int4 vs = va + vb;                  // (20,8,29,11)
    outputBuffer[14] = vs.x + vs.y;     // 28
    bool4 cmp = va < vb;                // (true,true,true,true)
    outputBuffer[15] = (cmp.x && cmp.w) ? 1 : 0; // 1

    // CHECK: 10
    // CHECK: 4
    // CHECK: 21
    // CHECK: 2
    // CHECK: 1
    // CHECK: 1
    // CHECK: 0
    // CHECK: 3
    // CHECK: 7
    // CHECK: 4
    // CHECK: 56
    // CHECK: 0
    // CHECK: -7
    // CHECK: -8
    // CHECK: 28
    // CHECK: 1
}
