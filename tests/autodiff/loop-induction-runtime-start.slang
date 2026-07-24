// Regression test for shader-slang/slang#12070 (escalated from slangpy#1051).
//
// bwd_diff of a [Differentiable] function containing a loop whose induction START value is a
// non-constant runtime expression (e.g. `-radius`, `radius`, where `radius` is a no_diff runtime
// value) used to miscompile: the reverse pass reconstructed the induction variable as
// `diffCounter * factor + counterOffset` and referenced `counterOffset` directly, even though for a
// runtime start that inst lives only in the primal (forward) function's scope. The result was an
// orphan-global SPIR-V ICE (`neg(<null>)` / `param`) and a used-before-def SIGSEGV on CUDA/HLSL/Metal.
//
// The fix materializes the loop-invariant `counterOffset` in the reverse scope before use, so the
// runtime-start form now produces the same gradients as the mathematically-identical zero-based form.
// Each runtime-start function below is paired with a zero-based twin; the two must agree (and match
// the hand-computed literal). `radius` is loaded from a buffer so it is genuinely runtime (a constant
// literal would be folded and would not exercise the non-constant `counterOffset` path).

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -shaderobj -output-using-type

//TEST_INPUT: set radiusBuffer = ubuffer(data=[2], stride=4)
RWStructuredBuffer<int> radiusBuffer;

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

// Negative runtime start: dx = -radius .. radius. counterOffset = neg(radius) (runtime).
// f = sum_{dx=-r..r} x*dx*dx  =>  df/dx = sum_{dx=-r..r} dx*dx.
[Differentiable]
float negStart(float x, no_diff int radius)
{
    float acc = 0.0;
    [MaxIters(17)]
    for (int dx = -radius; dx <= radius; ++dx)
        acc += x * float(dx * dx);
    return acc;
}

// Zero-based twin of negStart (constant start => always compiled correctly).
[Differentiable]
float negStartZeroBased(float x, no_diff int radius)
{
    float acc = 0.0;
    [MaxIters(17)]
    for (int t = 0; t < 2 * radius + 1; ++t)
    {
        int dx = t - radius;
        acc += x * float(dx * dx);
    }
    return acc;
}

// Bare runtime-param start: dx = radius .. 0. counterOffset = radius (the runtime param itself).
// f = sum_{dx=0..r} x*dx  =>  df/dx = sum_{dx=0..r} dx.
[Differentiable]
float paramStart(float x, no_diff int radius)
{
    float acc = 0.0;
    [MaxIters(17)]
    for (int dx = radius; dx >= 0; --dx)
        acc += x * float(dx);
    return acc;
}

// Zero-based twin of paramStart.
[Differentiable]
float paramStartZeroBased(float x, no_diff int radius)
{
    float acc = 0.0;
    [MaxIters(17)]
    for (int t = 0; t <= radius; ++t)
    {
        int dx = t;
        acc += x * float(dx);
    }
    return acc;
}

// Runtime start AND counterFactor != 1: dx = -radius, -radius+2, ... <= radius. counterOffset =
// neg(radius) (runtime), counterFactor = 2. f = sum x*dx*dx over that sequence.
[Differentiable]
float negStartStep2(float x, no_diff int radius)
{
    float acc = 0.0;
    [MaxIters(17)]
    for (int dx = -radius; dx <= radius; dx += 2)
        acc += x * float(dx * dx);
    return acc;
}

// Zero-based twin of negStartStep2 (same dx sequence, constant start).
[Differentiable]
float negStartStep2ZeroBased(float x, no_diff int radius)
{
    float acc = 0.0;
    [MaxIters(17)]
    for (int i = 0; i <= radius; ++i)
    {
        int dx = -radius + 2 * i;
        acc += x * float(dx * dx);
    }
    return acc;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    int radius = radiusBuffer[0]; // runtime value (== 2)

    // f is linear in x (f = x*C), so df/dx == C independent of x; use x = 1.0, seed = 1.0.

    // radius=2: sum_{dx=-2..2} dx*dx = 4+1+0+1+4 = 10.
    {
        var p = diffPair(1.0);
        bwd_diff(negStart)(p, radius, 1.0);
        outputBuffer[0] = p.d;
    }
    // CHECK: 10.0
    {
        var p = diffPair(1.0);
        bwd_diff(negStartZeroBased)(p, radius, 1.0);
        outputBuffer[1] = p.d;
    }
    // CHECK: 10.0

    // radius=2: sum_{dx=0..2} dx = 0+1+2 = 3.
    {
        var p = diffPair(1.0);
        bwd_diff(paramStart)(p, radius, 1.0);
        outputBuffer[2] = p.d;
    }
    // CHECK: 3.0
    {
        var p = diffPair(1.0);
        bwd_diff(paramStartZeroBased)(p, radius, 1.0);
        outputBuffer[3] = p.d;
    }
    // CHECK: 3.0

    // radius=2, step 2: dx in {-2,0,2}: sum dx*dx = 4+0+4 = 8.
    {
        var p = diffPair(1.0);
        bwd_diff(negStartStep2)(p, radius, 1.0);
        outputBuffer[4] = p.d;
    }
    // CHECK: 8.0
    {
        var p = diffPair(1.0);
        bwd_diff(negStartStep2ZeroBased)(p, radius, 1.0);
        outputBuffer[5] = p.d;
    }
    // CHECK: 8.0
}
