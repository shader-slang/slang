// Intrinsics whose vector or matrix overloads map to a native target operation compute vectors
// and matrices larger than 4 element by element, on every target. `frac` on a matrix with more
// than 4 rows runs its row loop past the old unroll budget of 4.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-metal -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-wgpu -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

typealias Vec8 = vector<float, 8>;
typealias Mat6 = matrix<float, 6, 6>;

[numthreads(1, 1, 1)]
void computeMain()
{
    Vec8 a; // 0.5, 1.0, ..., 4.0
    Vec8 b; // -3, -2, ..., 4
    Vec8 e0 = Vec8(0.0);
    for (int i = 0; i < 8; i++)
    {
        a[i] = float(i + 1) * 0.5;
        b[i] = float(i - 3);
    }
    e0[0] = 1.0;

    // CHECK: {{^}}16.000000
    outputBuffer[0] = ldexp(a, Vec8(2.0))[7];
    // CHECK-NEXT: {{^}}8.000000
    outputBuffer[1] = ldexp(a, vector<int, 8>(3))[1];
    // CHECK-NEXT: {{^}}3.141593
    outputBuffer[2] = radians(Vec8(180.0))[6];
    // dot(ng, i) = 8 >= 0 gives -n; with -ng it gives n.
    // CHECK-NEXT: {{^}}9.000000
    outputBuffer[3] = faceforward(Vec8(1.0), Vec8(1.0), Vec8(1.0))[3] +
                      10.0 * faceforward(Vec8(1.0), Vec8(1.0), Vec8(-1.0))[3];
    // reflect((1, ..., 1), e0) = (-1, 1, ..., 1).
    // CHECK-NEXT: {{^}}-9.000000
    outputBuffer[4] = reflect(Vec8(1.0), e0)[0] * 10.0 + reflect(Vec8(1.0), e0)[7];
    // refract(-e0, e0, 0.5): k = 1, so 0.5 * -e0 - (0.5 * -1 + 1) * e0 = -e0.
    // CHECK-NEXT: {{^}}-1.000000
    outputBuffer[5] = refract(-e0, e0, 0.5)[0] + refract(-e0, e0, 0.5)[7];

    Vec8 s;
    Vec8 c;
    sincos(Vec8(0.0), s, c);
    // CHECK-NEXT: {{^}}2.000000
    outputBuffer[6] = s[7] + 2.0 * c[7];
    Mat6 ms;
    Mat6 mc;
    sincos(Mat6(0.0), ms, mc);
    // CHECK-NEXT: {{^}}3.000000
    outputBuffer[7] = ms[5][5] + 3.0 * mc[5][4];

    // CHECK-NEXT: {{^}}0.250000
    outputBuffer[8] = frac(matrix<float, 12, 12>(2.25))[11][3];
    // CHECK-NEXT: {{^}}2.000000
    outputBuffer[9] = fabs(Vec8(-2.0))[7];
    // copysign(3, b) = (-3, ..., 3).
    // CHECK-NEXT: {{^}}-27.000000
    outputBuffer[10] = copysign(Vec8(3.0), b)[0] * 10.0 + copysign(Vec8(3.0), b)[7];
    // CHECK-NEXT: {{^}}9.000000
    outputBuffer[11] = cospi(Vec8(1.0))[5] + 10.0 * sinpi(Vec8(0.5))[6] + tanpi(Vec8(0.0))[7];
    // Metal's exp10(1.0f) is 9.999999, so the exp10 result is rounded. Rounding still tells
    // exp10 (10) apart from exp (3) and exp2 (2).
    // CHECK-NEXT: {{^}}13.000000
    outputBuffer[12] = divide(Vec8(9.0), Vec8(3.0))[4] + round(exp10(Vec8(1.0))[5]);
    // CHECK-NEXT: {{^}}8.000000
    outputBuffer[13] = powr(Vec8(2.0), Vec8(3.0))[2];

    Mat6 m = Mat6(1.0);
    m[5][5] = -0.5;
    // Only m[5][5] decides both: all(m > 0) is false and any(m < 0) is true. A reduction that
    // skips the last row or column would give 1.
    // CHECK-NEXT: {{^}}10.000000
    outputBuffer[14] = (all(m > 0.0) ? 1.0 : 0.0) + (any(m < 0.0) ? 10.0 : 0.0);
    // CHECK-NEXT: {{^}}3.141593
    outputBuffer[15] = radians(Mat6(180.0))[5][5];
}
