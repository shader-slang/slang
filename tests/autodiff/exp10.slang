//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-output-using-type -cpu
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[Differentiable]
float scalarExp10(float value)
{
    return exp10(value);
}

[Differentiable]
float2 vectorExp10(float2 value)
{
    return exp10(value);
}

[Differentiable]
float2x2 matrixExp10(float2x2 value)
{
    return exp10(value);
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    let scalarForward = fwd_diff(scalarExp10)(diffPair(0.0, 1.0));
    outputBuffer[0] = scalarForward.p; // CHECK: 1.0
    outputBuffer[1] = scalarForward.d; // CHECK: 2.302

    var scalarBackward = diffPair(0.0, 0.0);
    bwd_diff(scalarExp10)(scalarBackward, 1.0);
    outputBuffer[2] = scalarBackward.d; // CHECK: 2.302

    let vectorForward =
        fwd_diff(vectorExp10)(diffPair(float2(0.0, 1.0), float2(1.0, 2.0)));
    outputBuffer[3] = vectorForward.d.x; // CHECK: 2.302
    outputBuffer[4] = vectorForward.d.y; // CHECK: 46.05

    var vectorBackward = diffPair(float2(0.0, 1.0), float2(0.0));
    bwd_diff(vectorExp10)(vectorBackward, float2(1.0, 2.0));
    outputBuffer[5] = vectorBackward.d.x; // CHECK: 2.302
    outputBuffer[6] = vectorBackward.d.y; // CHECK: 46.05

    let matrixForward = fwd_diff(matrixExp10)(
        diffPair(float2x2(-1.0, 0.0, 1.0, 2.0), float2x2(1.0, 2.0, 3.0, 4.0)));
    outputBuffer[7] = matrixForward.d[0][0];  // CHECK: 0.230
    outputBuffer[8] = matrixForward.d[0][1];  // CHECK: 4.605
    outputBuffer[9] = matrixForward.d[1][0];  // CHECK: 69.07
    outputBuffer[10] = matrixForward.d[1][1]; // CHECK: 921.0

    var matrixBackward = diffPair(float2x2(-1.0, 0.0, 1.0, 2.0), float2x2(0.0));
    bwd_diff(matrixExp10)(matrixBackward, float2x2(1.0, 2.0, 3.0, 4.0));
    outputBuffer[11] = matrixBackward.d[0][0]; // CHECK: 0.230
    outputBuffer[12] = matrixBackward.d[0][1]; // CHECK: 4.605
    outputBuffer[13] = matrixBackward.d[1][0]; // CHECK: 69.07
    outputBuffer[14] = matrixBackward.d[1][1]; // CHECK: 921.0
}
