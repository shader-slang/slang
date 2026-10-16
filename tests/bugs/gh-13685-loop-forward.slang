// Regression test for https://github.com/shader-slang/slang/issues/13685
//
// A loop-header param can receive a defined value from the loop entry and itself
// from the back edge. Here the inner `if` merges an undefined value with the
// header param, and once that merge is forwarded, the header param's incoming
// values are `x` and the param itself. The param is then `x` on every path that
// does not take the `if`, and the result must be `x`.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

//TEST_INPUT: set inputBuffer = ubuffer(data=[7 4 -1], stride=4)
RWStructuredBuffer<int> inputBuffer;

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

int keepUnlessReplaced(int x, int n, int c)
{
    int t = x;
    for (int i = 0; i < n; i++)
    {
        if (c == i)
        {
            int r;
            t = r;
        }
    }
    return t;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    int x = inputBuffer[0];
    int n = inputBuffer[1];
    int c = inputBuffer[2];

    // CHECK: type: int32_t
    // CHECK-NEXT: 7
    // CHECK-NEXT: 7
    outputBuffer[0] = keepUnlessReplaced(x, n, c);
    outputBuffer[1] = keepUnlessReplaced(x, 0, 0);
}
