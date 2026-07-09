//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -emit-spirv-directly -output-using-type -capability spvBFloat16KHR -render-features bfloat16
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0

#pragma warning(disable: 41012)

// Element-wise arithmetic on CoopMat<BFloat16, ...>.
// Each entry fills two MatrixA fragments with uniform bf16 scalars, applies the
// operator, and uses .equals() against a known-expected matrix to produce a 1/0
// flag per operation. All flags should be 1 on every supported target.

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

typealias BMat = CoopMat<BFloat16, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixA>;

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain()
{
    let m1   = BMat(BFloat16(1.0));
    let m2   = BMat(BFloat16(2.0));
    let m3   = BMat(BFloat16(3.0));
    let m5   = BMat(BFloat16(5.0));
    let m6   = BMat(BFloat16(6.0));
    let m7   = BMat(BFloat16(7.0));
    let mNeg3 = BMat(BFloat16(-3.0));

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
