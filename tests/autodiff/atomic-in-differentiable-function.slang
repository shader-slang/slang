// An atomic operation inside a differentiated function is a non-differentiable
// side effect: forward and backward differentiation keep it in the primal
// computation and propagate derivatives around it, as they do for a store to
// the same buffer. How many times the side effect runs inside a derivative
// function is not guaranteed, so only the derivatives are checked.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

//TEST_INPUT: set counters = ubuffer(data=[0 0], stride=4)
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
}

// d/dx x^2 at 3, forward and backward.
// CHECK: 6.0
// CHECK-NEXT: 6.0
// d/dx x^3 at 2, forward and backward.
// CHECK-NEXT: 12.0
// CHECK-NEXT: 12.0
