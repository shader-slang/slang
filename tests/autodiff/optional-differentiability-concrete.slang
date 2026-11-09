// TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
// TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type

[Differentiable] float squareWithMetadata<T>(T metadata, float value) where optional T
    : IDifferentiable
{
    return value * value;
}

[Differentiable]
float squareWithMetadataPack<each T>(float value, expand each T metadata)
    where optional T : IDifferentiable
{
    return value * value;
}

// TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)] void computeMain()
{
    var x = diffPair(3.0);
    bwd_diff(squareWithMetadata<uint>)(7u, x, 1.0);
    outputBuffer[0] = x.d;                                                          // CHECK: 6.0
    outputBuffer[1] = fwd_diff(squareWithMetadata<uint>)(7u, diffPair(3.0, 1.0)).d; // CHECK: 6.0

    var metadata = diffPair(7.0);
    x = diffPair(3.0);
    bwd_diff(squareWithMetadata<float>)(metadata, x, 1.0);
    outputBuffer[2] = metadata.d; // CHECK: 0.0
    outputBuffer[3] = x.d;        // CHECK: 6.0

    x = diffPair(3.0);
    metadata = diffPair(7.0);
    bwd_diff(squareWithMetadataPack<uint, float>)(x, 7u, metadata, 1.0);
    outputBuffer[4] = x.d;        // CHECK: 6.0
    outputBuffer[5] = metadata.d; // CHECK: 0.0
    outputBuffer[6] = fwd_diff(squareWithMetadataPack<uint, float>)(
        diffPair(3.0, 1.0), 7u, diffPair(7.0, 1.0)).d; // CHECK: 6.0
}
