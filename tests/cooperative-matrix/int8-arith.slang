//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0

// Element-wise arithmetic on CoopMat<int8_t, ...>. Each entry fills two MatrixA
// fragments with uniform int8 scalars, applies the operator, and uses .equals()
// against a known-expected matrix to produce a 1/0 flag per operation. All flags
// should be 1.
//
// Storing a `CoopMat<int8_t, MatrixA>` is not round-trip-correct on CUDA today
// (the store path uses the MatrixC register layout — see the bfloat16 work),
// so we sidestep Store via the .equals() flag pattern.

// CHECK: type: uint
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

using namespace linalg;

typealias IMat = CoopMat<int8_t, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixA>;

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain()
{
    let m1   = IMat(int8_t(1));
    let m2   = IMat(int8_t(2));
    let m3   = IMat(int8_t(3));
    let m5   = IMat(int8_t(5));
    let m6   = IMat(int8_t(6));
    let m7   = IMat(int8_t(7));
    let mNeg3 = IMat(int8_t(-3));

    // 3 + 2 == 5
    outputBuffer[0] = (m3 + m2).equals(m5)    ? 1 : 0;
    // 5 - 2 == 3
    outputBuffer[1] = (m5 - m2).equals(m3)    ? 1 : 0;
    // 3 * 2 == 6
    outputBuffer[2] = (m3 * m2).equals(m6)    ? 1 : 0;
    // 6 / 2 == 3
    outputBuffer[3] = (m6 / m2).equals(m3)    ? 1 : 0;
    // -3
    outputBuffer[4] = (-m3).equals(mNeg3)     ? 1 : 0;
    // 7 % 2 == 1
    outputBuffer[5] = (m7 % m2).equals(m1)    ? 1 : 0;
}
