// This test intentionally declares only a CPU run. It takes the address of a local `Sphere` to
// exercise `sphere->distance`, but local pointer values are not portable GPU shader inputs or
// storage. The test harness can still synthesize compatible variants such as CUDA from this run.
// Vulkan render execution fails before producing output for this shape, so a Vulkan directive
// would not validate the gradient. Keeping this case separate lets the non-pointer receiver cases
// run across all supported backends.
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

struct Sphere
{
    float r;

    [Differentiable]
    [NoDiffThis]
    float distance(float3 p)
    {
        return length(p) - r;
    }
}

float3 pointerGradient(Sphere* sphere, float3 p)
{
    var dp = diffPair(p);

    // The base of `sphere->distance` has type `Ptr<Sphere>`, but the method receives its
    // pointee as `this`, so the static derivative's explicit `this` argument is `*sphere`.
    bwd_diff(sphere->distance)(*sphere, dp, 1.0);
    return dp.d;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    Sphere sphere = {1.0};
    float3 gradient = pointerGradient(&sphere, float3(3.0, 0.0, 0.0));

    outputBuffer[0] = gradient.x;
    outputBuffer[1] = gradient.y;
    outputBuffer[2] = gradient.z;

    // CHECK: 1.0
    // CHECK: 0.0
    // CHECK: 0.0
}
