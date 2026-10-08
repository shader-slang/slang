// Builtin-operator fast-path coverage for bool operands: equality (`==`/`!=`) and unary `!`,
// on scalar bool and bool vectors. (`&&`/`||` deliberately stay on the short-circuit path and
// are not part of the fast path.) All operands are runtime values so nothing folds away.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-d3d12 -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    bool p = (outputBuffer[0] + 7) > (outputBuffer[1] + 3); // 7 > 3  -> true
    bool q = (outputBuffer[2] + 1) == (outputBuffer[3] + 2); // 1 == 2 -> false

    outputBuffer[0] = (p == q) ? 1 : 0; // true == false -> false -> 0
    outputBuffer[1] = (p != q) ? 1 : 0; // true != false -> true  -> 1
    outputBuffer[2] = (!p) ? 1 : 0;     // !true  -> 0
    outputBuffer[3] = (!q) ? 1 : 0;     // !false -> 1

    // Bool vector equality (component-wise -> vector<bool,4> in non-GLSL mode).
    bool4 bp = bool4(p, q, p, q); // (T,F,T,F)
    bool4 bq = bool4(q, q, p, p); // (F,F,T,T)
    bool4 beq = bp == bq;         // (F,T,T,F)
    outputBuffer[4] = beq.x ? 1 : 0; // 0
    outputBuffer[5] = beq.y ? 1 : 0; // 1
    outputBuffer[6] = beq.z ? 1 : 0; // 1
    outputBuffer[7] = beq.w ? 1 : 0; // 0

    // CHECK: 0
    // CHECK: 1
    // CHECK: 0
    // CHECK: 1
    // CHECK: 0
    // CHECK: 1
    // CHECK: 1
    // CHECK: 0
}
