// Verify that an atomic's out argument receives the original value after lowering copies
// it back from a temporary with its derivative detached. Integer atomics exercise the same
// rewrite as float atomics without requiring float-atomic support on the device.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-mtl -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

//TEST_INPUT: set counters = ubuffer(data=[4], stride=4)
RWStructuredBuffer<uint> counters;

[Differentiable]
float scaledByAtomicOutput(float x)
{
    // Add zero so the original value stays four regardless of how often autodiff executes
    // the side effect. The nonzero initializer also exposes a missing copy-back to original.
    uint original = 99u;
    InterlockedAdd(counters[0], 0u, original);
    return x * float(original + 1u);
}

[numthreads(1, 1, 1)]
void computeMain()
{
    let forward = fwd_diff(scaledByAtomicOutput)(diffPair(2.0f, 1.0f));
    outputBuffer[0] = forward.p;
    outputBuffer[1] = forward.d;

    var backward = diffPair(2.0f, 0.0f);
    bwd_diff(scaledByAtomicOutput)(backward, 1.0f);
    outputBuffer[2] = backward.d;
}

// The primal is 2 * (4 + 1); the derivative is 4 + 1 in both differentiation modes.
// CHECK: 10.0
// CHECK-NEXT: 5.0
// CHECK-NEXT: 5.0
