//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -use-dxil
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-mtl
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda

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
    c = and(a, b);

    bool1 i, j, k;
    bool2 l, m, n;
    bool3 o, p, q;
    bool4 r, s, t;
    k = and(i, j);
    n = and(m, l);
    q = and(o, p);
    t = and(r, s);

    k = !and(k, false);
    n = !and(n, false);
    q = !and(q, false);
    t = !and(t, false);

    outputBuffer[tid] = all(k) && all(n) && all(q) && all(t);
}
