//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -use-dxil
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-mtl
//DISABLE_TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu
//DISABLE_TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda

// CHECK:      1
// CHECK-NEXT: 1
// CHECK-NEXT: 1
// CHECK-NEXT: 1

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint tid : SV_GroupIndex)
{
    bool a, b, c;
    c = or(a, b);

    bool1 i, j, k;
    bool2 l, m, n;
    bool3 o, p, q;
    bool4 r, s, t;
    k = or(i, j);
    n = or(m, l);
    q = or(o, p);
    t = or(r, s);

    k = or(k, true);
    n = or(n, true);
    q = or(q, true);
    t = or(t, true);

    outputBuffer[tid] = all(k) && all(n) && all(q) && all(t);
}
