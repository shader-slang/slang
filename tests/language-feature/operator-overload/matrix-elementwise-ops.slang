//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type

// Same-shape matrix products, scalar products, and matrix addition stay element-wise now that
// the matrix-vector `*` shapes have diagnostic fallback overloads (#14546). With m2 equal to
// twice the identity, the element-wise product has zero off-diagonals while the algebraic
// product `mul(m1, m2)` would carry them, so the values below pin the element-wise reading.

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    float a = outputBuffer[0] + 1.0;                    // 1.0
    float2x2 m1 = float2x2(a, 2.0 * a, 3.0 * a, 4.0 * a); // {{1,2},{3,4}}
    float2x2 m2 = float2x2(2.0 * a, 0.0, 0.0, 2.0 * a);   // 2 * identity

    float2x2 prod = m1 * m2; // element-wise {{2,0},{0,8}}; algebraic would be {{2,4},{6,8}}
    outputBuffer[1] = prod[0][1] + prod[1][0]; // 0.0 (algebraic would give 10.0)
    outputBuffer[2] = prod[0][0];             // 2.0
    outputBuffer[3] = prod[1][1];             // 8.0

    // Scalar products broadcast element-wise.
    float2x2 scaled = m1 * 2.0;
    float2x2 scaledLeft = 0.5 * m1;
    outputBuffer[4] = scaled[1][0] + scaledLeft[1][0]; // 6.0 + 1.5 = 7.5

    // Matrix addition is element-wise.
    float2x2 sum = m1 + m2;
    outputBuffer[0] = sum[0][0]; // 3.0

    // CHECK: 3.0
    // CHECK: 0.0
    // CHECK: 2.0
    // CHECK: 8.0
    // CHECK: 7.5
}
