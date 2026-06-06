// Regression test: a `for` loop with a builtin comparison predicate (`i < N`) inside a
// `[Differentiable]` function. The builtin-operator fast path leaves the predicate's
// comparison operator unresolved, so the for-loop max-iterations inference
// (tryInferLoopMaxIterations) must recognize the fast-path node and still infer the
// iteration count; otherwise reverse-mode auto-diff fails with
// "loop in differentiable function needs attributes" (E30510).

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

[Differentiable]
float poly(float x)
{
    float acc = 0.0;
    // `i < 8` is a fast-pathed builtin comparison; the loop must still infer [MaxIters(8)].
    for (int i = 0; i < 8; i++)
    {
        acc = acc + x * x;
    }
    return acc;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    float x = 3.0;
    // f(x) = 8 * x^2 = 72 at x=3
    outputBuffer[0] = poly(x);

    // f'(x) = 16 * x = 48 at x=3
    var dp = diffPair(x, 0.0);
    bwd_diff(poly)(dp, 1.0);
    outputBuffer[1] = dp.d;

    // CHECK: 72.0
    // CHECK: 48.0
}
