// Vectors and matrices with more than 4 elements, rows or columns are lowered to arrays.
// Check the core operations on a 12x12 matrix and an 8-wide vector at runtime.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-metal -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-wgpu -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

typealias Mat12 = matrix<float, 12, 12>;
typealias Vec12 = vector<float, 12>;
typealias Vec8 = vector<float, 8>;

[numthreads(1, 1, 1)]
void computeMain()
{
    Mat12 a;
    Mat12 b;
    Vec12 v;
    for (int i = 0; i < 12; i++)
    {
        v[i] = float(i + 1);
        for (int j = 0; j < 12; j++)
        {
            a[i][j] = float(i - j);
            b[i][j] = float((i * j) % 5);
        }
    }

    // matrix-matrix, matrix-vector and vector-matrix products.
    Mat12 c = mul(a, b);
    // CHECK: {{^}}-56.000000
    outputBuffer[0] = c[3][7];
    Vec12 mv = mul(a, v);
    // CHECK-NEXT: {{^}}-182.000000
    outputBuffer[1] = mv[5];
    Vec12 vm = mul(v, a);
    // CHECK-NEXT: {{^}}182.000000
    outputBuffer[2] = vm[5];

    // transpose, dot, and element access with a dynamic index.
    Mat12 t = transpose(a);
    int k = int(v[1]); // 2, not a compile-time constant after the loop above
    // CHECK-NEXT: {{^}}7.000000
    outputBuffer[3] = t[k][9];
    // CHECK-NEXT: {{^}}650.000000
    outputBuffer[4] = dot(v, v);

    // Component-wise arithmetic with matrix and scalar operands.
    Mat12 d = a + b * 2.0 - 1.0;
    // CHECK-NEXT: {{^}}5.000000
    outputBuffer[5] = d[4][6];

    // An 8-wide vector: construction, element-wise intrinsics, comparison and select.
    Vec8 w = Vec8(1, 2, 3, 4, 5, 6, 7, 8);
    w = max(w * 2.0, Vec8(9.0));
    // CHECK-NEXT: {{^}}25.000000
    outputBuffer[6] = w[2] + w[7];
    vector<bool, 8> big = w > 10.0;
    Vec8 s = select(big, w, -w);
    // CHECK-NEXT: {{^}}-4.000000
    outputBuffer[7] = dot(s, Vec8(1.0));
    // CHECK-NEXT: {{^}}6.000000
    outputBuffer[8] = length(Vec8(abs(s.x) - 5.0, 0, 0, 0, 0, 0, 0, 0)) + clamp(s, -1.0, 1.0)[0] + 3.0;

    // Swizzle of the first four elements, and a whole-matrix comparison.
    // CHECK-NEXT: {{^}}31.000000
    outputBuffer[9] = dot(w.xyzw, float4(1, 1, 1, 1)) + sqrt(abs(Vec8(-4.0)))[3] - 9.0 + 2.0;
    matrix<bool, 12, 12> eq = (a == transpose(t));
    // CHECK-NEXT: {{^}}1.000000
    outputBuffer[10] = all(eq[7]) ? 1.0 : 0.0;
    // CHECK-NEXT: {{^}}1.000000
    outputBuffer[11] = any((a == transpose(-t))[0] != (a == transpose(-t))[11]) ? 1.0 : 0.0;
}
