// Regression/correctness test for the hard-coded builtin scalar/vector/matrix coercion fast path.
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    int seed = outputBuffer[0] + 3;

    float scalarValue = seed;
    float3 vectorFromScalar = seed;

    int3 intVector = int3(seed, seed + 1, seed + 2);
    float3 vectorFromVector = intVector;

    float2x2 matrixFromScalar = seed;

    int2x2 intMatrix = seed;
    float2x2 matrixFromMatrix = intMatrix;

    vector<int, 1> oneVector = seed + 4;
    int scalarFromOneVector = oneVector;

    outputBuffer[0] = (int)scalarValue;
    outputBuffer[1] = (int)(vectorFromScalar.x + vectorFromScalar.y + vectorFromScalar.z);
    outputBuffer[2] = (int)(vectorFromVector.x + vectorFromVector.y + vectorFromVector.z);
    outputBuffer[3] = (int)(matrixFromScalar[0][0] + matrixFromScalar[1][1]);
    outputBuffer[4] = (int)(matrixFromMatrix[0][1] + matrixFromMatrix[1][0]);
    outputBuffer[5] = scalarFromOneVector;

    // CHECK: 3
    // CHECK: 9
    // CHECK: 12
    // CHECK: 6
    // CHECK: 6
    // CHECK: 7
}
