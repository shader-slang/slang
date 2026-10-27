// Test that `-inline-single-call-functions` keeps results unchanged when it
// inlines a wrapper chain with out/inout parameters, early returns, and
// a loop.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -xslang -inline-single-call-functions
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

int twice(int x) { return x * 2; }

// Early return and a loop in the inlined body.
int body(int n, out int count)
{
    count = 0;
    if (n < 0)
        return -1;
    int sum = 0;
    for (int i = 0; i < n; i++)
    {
        sum += twice(i);
        count++;
    }
    return sum;
}

int wrap0(int n, inout int extra)
{
    int count;
    int r = body(n, count);
    extra += count;
    return r + twice(1);
}

int wrap1(int n, inout int extra) { return wrap0(n, extra) + 1; }

[numthreads(4, 1, 1)]
void computeMain(int3 tid: SV_DispatchThreadID)
{
    int extra = 100;
    int r = wrap1(int(tid.x) * 2 - 1, extra);
    outputBuffer[tid.x] = r * 1000 + extra;
}

// n = -1: body returns -1, count 0 -> r = -1 + 2 + 1 = 2, extra 100
// CHECK: 2100
// n = 1: sum 0, count 1 -> r = 0 + 3 = 3, extra 101
// CHECK: 3101
// n = 3: sum 0+2+4 = 6, count 3 -> r = 9, extra 103
// CHECK: 9103
// n = 5: sum 20, count 5 -> r = 23, extra 105
// CHECK: 23105
