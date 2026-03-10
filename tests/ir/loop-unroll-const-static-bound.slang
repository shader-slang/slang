//TEST:SIMPLE(filecheck=CHECK): -entry printMain -target hlsl -profile cs_6_3 -O3

struct test
{
    int a;
    int b;
    bool c;

    int value() { return c ? b : a; }
};

struct globals
{
    int testcount;
};

RWStructuredBuffer<int> results;

static const globals globalsbuffer[1] = { { 2 } };
static test tests[2] = { { 5, 2, true }, { 10, 20, false } };

[shader("compute")]
[numthreads(1, 1, 1)]
void printMain()
{
    // CHECK-NOT: globalsbuffer
    // CHECK: results_0[int(0)] = results_0[int(0)] + test_value_0(tests_0[0U]);
    // CHECK: results_0[int(0)] = results_0[int(0)] + test_value_0(tests_0[1U]);
    [ForceUnroll]
    for (uint nTestIndex = 0; nTestIndex < globalsbuffer[0].testcount; nTestIndex++)
    {
        results[0] += tests[nTestIndex].value();
    }
}
