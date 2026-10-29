// Large vectors and matrices of other shapes and element types: an 8x3 matrix (native rows), a
// 3x8 matrix, integer and boolean elements, casts, swizzled stores, and initializer lists.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-metal -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-wgpu -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

struct Particle
{
    vector<int, 6> ids;
    matrix<float, 8, 3> frames;
}

int sumOf<let N : int>(vector<int, N> v)
{
    int s = 0;
    for (int i = 0; i < N; i++)
        s += v[i];
    return s;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    // 8x3 (rows stay native float3) times 3x8.
    matrix<float, 8, 3> tall;
    matrix<float, 3, 8> wide;
    for (int i = 0; i < 8; i++)
    {
        tall[i] = float3(i, 1, -i);
        for (int j = 0; j < 3; j++)
            wide[j][i] = float(i * j);
    }
    matrix<float, 8, 8> sq = mul(tall, wide);
    // sq[r][c] = r*0*c + 1*1*c + (-r)*2*c = c - 2rc; r=3, c=5: 5 - 30
    // CHECK: {{^}}-25
    outputBuffer[0] = int(sq[3][5]);
    matrix<float, 3, 3> small = mul(wide, tall);
    // small[1][0] = sum_i (i*1) * i = 140
    // CHECK-NEXT: {{^}}140
    outputBuffer[1] = int(small[1][0]);

    // Integer vectors, casts and bit operations.
    vector<int, 6> a = { 1, 2, 3, 4, 5, 6 };
    vector<int, 6> b = (a << 1) | 1;
    vector<float, 6> f = vector<float, 6>(b) * 0.5;
    vector<int, 6> c = vector<int, 6>(f + 0.5);
    // b = 3 5 7 9 11 13, c = 2 3 4 5 6 7
    // CHECK-NEXT: {{^}}27
    outputBuffer[2] = sumOf(c);

    // Swizzled store, comparisons producing bool vectors, and a struct field.
    Particle p;
    p.ids = a;
    p.ids.yx = int2(10, 20);
    p.frames = tall;
    vector<bool, 6> odd = (p.ids % 2) == 1;
    // ids = 20 10 3 4 5 6 -> odd at 3 and 5
    // CHECK-NEXT: {{^}}2
    outputBuffer[3] = sumOf(select(odd, vector<int, 6>(1), vector<int, 6>(0)));
    // CHECK-NEXT: {{^}}30
    outputBuffer[4] = p.ids[0] + p.ids[1];
    // CHECK-NEXT: {{^}}-7
    outputBuffer[5] = int(p.frames[7].z);

    // Negation, min and abs on an integer vector, and a truncating cast to a native vector.
    vector<int, 6> n = -a;
    int4 head = int4(abs(min(n, vector<int, 6>(-2))).xyzw);
    // head = 2 2 3 4
    // CHECK-NEXT: {{^}}11
    outputBuffer[6] = head.x + head.y + head.z + head.w;
    // CHECK-NEXT: {{^}}1
    outputBuffer[7] = all(abs(n) == a) ? 1 : 0;
}
