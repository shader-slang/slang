//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -emit-spirv-directly -output-using-type -capability spvBFloat16KHR -render-features bfloat16
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0

#pragma warning(disable: 41012)

// Comparison operators on CoopMat<BFloat16, ...> (equals / lessThan /
// lessThanOrEquals). The matrices are filled with uniform values so every lane
// observes the same comparison result; each flag is 1 if the comparison is
// correct, 0 otherwise. All slots should be 1 on every supported target.

// CHECK: type: uint
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

using namespace linalg;

typealias BMat = CoopMat<BFloat16, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixA>;

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain()
{
    let m3a = BMat(BFloat16(3.0));
    let m3b = BMat(BFloat16(3.0));
    let m5  = BMat(BFloat16(5.0));

    // ==
    outputBuffer[0] = (m3a == m3b) ? 1 : 0;        // 3 == 3 -> true
    outputBuffer[1] = !(m3a == m5) ? 1 : 0;        // 3 != 5 -> true

    // <
    outputBuffer[2] = (m3a < m5)   ? 1 : 0;        // 3 < 5  -> true
    outputBuffer[3] = !(m5 < m3a)  ? 1 : 0;        // !(5<3) -> true
    outputBuffer[4] = !(m3a < m3b) ? 1 : 0;        // !(3<3) -> true

    // <=
    outputBuffer[5] = (m3a <= m5)  ? 1 : 0;        // 3 <= 5 -> true
    outputBuffer[6] = (m3a <= m3b) ? 1 : 0;        // 3 <= 3 -> true
    outputBuffer[7] = !(m5 <= m3a) ? 1 : 0;        // !(5<=3) -> true
}
