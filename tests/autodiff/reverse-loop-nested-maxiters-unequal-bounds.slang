//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// Regression test for https://github.com/shader-slang/slang/issues/13000:
// bwd_diff through two nested, non-unrolled [MaxIters] loops with unequal
// iteration counts must produce the correct gradient with respect to the
// function input; the [ForceUnroll] variant is the analytic control.
//
// Two non-obvious requirements to exercise the path:
//  - The loop bounds must DIFFER by enough to overrun the mis-sized dimension.
//    Empirically (see issue #13000) the wrong gradient appears only once the
//    bounds differ by 3 or more, so this test uses a difference of 4.
//  - The per-iteration weight must be CHECKPOINTED, so it is read from a
//    differentiable input array through a [Differentiable] accessor. An
//    inline-constant weight would be recomputed in reverse (never stored) and
//    would not exercise the per-iteration checkpoint array at all.
//
// Both bound orderings are covered, because they fail differently:
//  - outer > inner (kInner=3, kOuter=7): the mis-sized array ALIASES in-bounds
//    slots -> a deterministic wrong gradient.
//  - inner > outer (kInner=7, kOuter=3): the inner counter OVERRUNS the
//    (smaller) outer-sized dimension -> an out-of-bounds access (the CUDA
//    illegal-address manifestation).
// The [ForceUnroll] twins are unrolled before the autodiff pass (no checkpoint
// array) and pin the analytic answer for each orientation.

// Differentiable accessor: its per-iteration result is a checkpointed primal.
[Differentiable]
float getW(float W[21], int idx)
{
    return W[idx];
}

//
// Orientation A: outer loop (7) runs more than inner loop (3) -> in-bounds
// aliasing. dLoss/dx[col] = sum over 7 rows of W[row*3+col] = 63 + 7*(col+1).
//

[Differentiable]
float forwardA_maxIters(float x[3], float W[21])
{
    float loss = 0.0;
    [MaxIters(7)]
    for (int row = 0; row < 7; ++row)
    {
        float y = 0.0;
        [MaxIters(3)]
        for (int col = 0; col < 3; ++col)
            y += getW(W, row * 3 + col) * x[col];
        loss += y;
    }
    return loss;
}

[Differentiable]
float forwardA_unroll(float x[3], float W[21])
{
    float loss = 0.0;
    [ForceUnroll]
    for (int row = 0; row < 7; ++row)
    {
        float y = 0.0;
        [ForceUnroll]
        for (int col = 0; col < 3; ++col)
            y += getW(W, row * 3 + col) * x[col];
        loss += y;
    }
    return loss;
}

//
// Orientation B: inner loop (7) runs more than outer loop (3) -> overrun.
// dLoss/dx[col] = sum over 3 rows of W[row*7+col] = 24 + 3*col.
//

[Differentiable]
float forwardB_maxIters(float x[7], float W[21])
{
    float loss = 0.0;
    [MaxIters(3)]
    for (int row = 0; row < 3; ++row)
    {
        float y = 0.0;
        [MaxIters(7)]
        for (int col = 0; col < 7; ++col)
            y += getW(W, row * 7 + col) * x[col];
        loss += y;
    }
    return loss;
}

[Differentiable]
float forwardB_unroll(float x[7], float W[21])
{
    float loss = 0.0;
    [ForceUnroll]
    for (int row = 0; row < 3; ++row)
    {
        float y = 0.0;
        [ForceUnroll]
        for (int col = 0; col < 7; ++col)
            y += getW(W, row * 7 + col) * x[col];
        loss += y;
    }
    return loss;
}

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    float W[21];
    [ForceUnroll]
    for (int i = 0; i < 21; ++i)
        W[i] = float(i + 1);

    // dx is the column-sum of W and is independent of x, so x is arbitrary.
    float xA[3];
    [ForceUnroll]
    for (int i = 0; i < 3; ++i)
        xA[i] = 1.0;

    float xB[7];
    [ForceUnroll]
    for (int i = 0; i < 7; ++i)
        xB[i] = 1.0;

    var dpA = diffPair(xA);
    var dpAW = diffPair(W);
    bwd_diff(forwardA_maxIters)(dpA, dpAW, 1.0);

    var dpAu = diffPair(xA);
    var dpAuW = diffPair(W);
    bwd_diff(forwardA_unroll)(dpAu, dpAuW, 1.0);

    var dpB = diffPair(xB);
    var dpBW = diffPair(W);
    bwd_diff(forwardB_maxIters)(dpB, dpBW, 1.0);

    var dpBu = diffPair(xB);
    var dpBuW = diffPair(W);
    bwd_diff(forwardB_unroll)(dpBu, dpBuW, 1.0);

    // Orientation A: dx = 63 + 7*(col+1) = {70, 77, 84}.
    outputBuffer[0] = dpA.d[0];
    // CHECK: 70.0
    outputBuffer[1] = dpA.d[1];
    // CHECK: 77.0
    outputBuffer[2] = dpA.d[2];
    // CHECK: 84.0
    outputBuffer[3] = dpAu.d[0];
    // CHECK: 70.0
    outputBuffer[4] = dpAu.d[1];
    // CHECK: 77.0
    outputBuffer[5] = dpAu.d[2];
    // CHECK: 84.0

    // Orientation B: dx = 24 + 3*col = {24, 27, 30, 33, 36, 39, 42}.
    outputBuffer[6] = dpB.d[0];
    // CHECK: 24.0
    outputBuffer[7] = dpB.d[1];
    // CHECK: 27.0
    outputBuffer[8] = dpB.d[2];
    // CHECK: 30.0
    outputBuffer[9] = dpB.d[3];
    // CHECK: 33.0
    outputBuffer[10] = dpB.d[4];
    // CHECK: 36.0
    outputBuffer[11] = dpB.d[5];
    // CHECK: 39.0
    outputBuffer[12] = dpB.d[6];
    // CHECK: 42.0
    outputBuffer[13] = dpBu.d[0];
    // CHECK: 24.0
    outputBuffer[14] = dpBu.d[1];
    // CHECK: 27.0
    outputBuffer[15] = dpBu.d[2];
    // CHECK: 30.0
    outputBuffer[16] = dpBu.d[3];
    // CHECK: 33.0
    outputBuffer[17] = dpBu.d[4];
    // CHECK: 36.0
    outputBuffer[18] = dpBu.d[5];
    // CHECK: 39.0
    outputBuffer[19] = dpBu.d[6];
    // CHECK: 42.0
}
