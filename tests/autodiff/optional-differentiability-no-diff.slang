//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

interface ITag {}

// Explicit no_diff inputs do not impose optional differentiation's parameter-mode restriction.
[Differentiable]
float withMetadata<T>(no_diff inout T metadata, float value)
    where optional T : IDifferentiable
{
    return value * value;
}

// An unrelated optional constraint does not change ordinary autodiff parameter modes.
[Differentiable]
float withUnrelatedConstraint<T>(T metadata, inout float value) where optional T : ITag
{
    value *= 2.0;
    return value;
}

//TEST_INPUT: set output = out ubuffer(data=[0 0], stride=4)
RWStructuredBuffer<float> output;

[numthreads(1, 1, 1)]
void computeMain()
{
    float metadata = 2.0;
    var value = diffPair(3.0);
    bwd_diff(withMetadata<float>)(metadata, value, 1.0);
    output[0] = value.d;
    // CHECK: 6.0

    value = diffPair(3.0);
    bwd_diff(withUnrelatedConstraint<uint>)(0u, value, 1.0);
    output[1] = value.d;
    // CHECK: 2.0
}
