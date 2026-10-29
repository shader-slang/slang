// An atomic operation inside a differentiated function is a non-differentiable
// side effect: forward and backward differentiation keep it in the primal
// computation and propagate derivatives around it, as they do for a store to
// the same buffer. How many times the side effect runs inside a derivative
// function is not guaranteed, so only the derivatives are checked.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-mtl -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0 0 0 0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

//TEST_INPUT: set counters = ubuffer(data=[0 0 4 0 0 0 0], stride=4)
RWStructuredBuffer<Atomic<uint>> counters;

[Differentiable]
float square(float x)
{
    counters[0].add(1u);
    return x * x;
}

// The atomic sits inside a loop, which backward differentiation reverses
// through its loop state.
[Differentiable]
float cube(float x)
{
    float y = 1.0f;
    for (uint i = 0; i < 3; i++)
    {
        counters[1].add(1u);
        y = y * x;
    }
    return y;
}

// The value an atomic returns is a primal constant for differentiation. Adding
// zero leaves counters[2] at its initial 4 however many times the derivative
// code runs the atomic, so the derivative is exactly 4 + 1.
[Differentiable]
float scaledByCounter(float x)
{
    uint original = counters[2].add(0u);
    return x * float(original + 1u);
}

// An atomic wrapped in `no_diff`.
[Differentiable]
float squareNoDiff(float x)
{
    no_diff(counters[3].add(1u));
    return x * x;
}

// Atomic operations other than `add` take the same path.
[Differentiable]
float tripleWithOtherAtomics(float x)
{
    counters[4].max(7u);
    counters[5].exchange(3u);
    counters[5].compareExchange(3u, 9u);
    counters[6].or(2u);
    return x * 3.0f;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = fwd_diff(square)(diffPair(3.0f, 1.0f)).d;
    var squarePair = diffPair(3.0f, 0.0f);
    bwd_diff(square)(squarePair, 1.0f);
    outputBuffer[1] = squarePair.d;

    outputBuffer[2] = fwd_diff(cube)(diffPair(2.0f, 1.0f)).d;
    var cubePair = diffPair(2.0f, 0.0f);
    bwd_diff(cube)(cubePair, 1.0f);
    outputBuffer[3] = cubePair.d;

    outputBuffer[4] = fwd_diff(scaledByCounter)(diffPair(2.0f, 1.0f)).d;
    var scaledPair = diffPair(2.0f, 0.0f);
    bwd_diff(scaledByCounter)(scaledPair, 1.0f);
    outputBuffer[5] = scaledPair.d;

    outputBuffer[6] = fwd_diff(squareNoDiff)(diffPair(4.0f, 1.0f)).d;
    var noDiffPair = diffPair(4.0f, 0.0f);
    bwd_diff(squareNoDiff)(noDiffPair, 1.0f);
    outputBuffer[7] = noDiffPair.d;

    outputBuffer[8] = fwd_diff(tripleWithOtherAtomics)(diffPair(1.0f, 1.0f)).d;
    var triplePair = diffPair(1.0f, 0.0f);
    bwd_diff(tripleWithOtherAtomics)(triplePair, 1.0f);
    outputBuffer[9] = triplePair.d;
}

// d/dx x^2 at 3, forward and backward.
// CHECK: 6.0
// CHECK-NEXT: 6.0
// d/dx x^3 at 2, forward and backward.
// CHECK-NEXT: 12.0
// CHECK-NEXT: 12.0
// d/dx x * (4 + 1), forward and backward.
// CHECK-NEXT: 5.0
// CHECK-NEXT: 5.0
// d/dx x^2 at 4, forward and backward.
// CHECK-NEXT: 8.0
// CHECK-NEXT: 8.0
// d/dx 3x, forward and backward.
// CHECK-NEXT: 3.0
// CHECK-NEXT: 3.0
