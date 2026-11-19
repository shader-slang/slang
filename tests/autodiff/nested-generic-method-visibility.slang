//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type -xslang -std -xslang 2026
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type

// Derivatives of public generic methods nested in several generic containers of a non-public
// module are usable from an importing module.

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

import "nested-generic-method-visibility-lib";

[Differentiable]
float callSquare(float x)
{
    Scaler s;
    return s.square(x, 0);
}

[Differentiable]
float callCube(float x)
{
    Outer<float>.Inner<int> inner;
    return inner.cube(x, 0);
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    outputBuffer[0] = fwd_diff(callSquare)(diffPair(3.0, 1.0)).d;
    outputBuffer[1] = fwd_diff(callCube)(diffPair(2.0, 1.0)).d;

    var dCube = diffPair(7.0);
    bwd_diff(callCube)(dCube, 1.0);
    outputBuffer[2] = dCube.d;

    outputBuffer[3] = fwd_diff(Scaler.squareStatic<int>)(diffPair(3.0, 1.0), 0).d;

    var dSquareStatic = diffPair(3.0);
    bwd_diff(Scaler.squareStatic<int>)(dSquareStatic, 0, 1.0);
    outputBuffer[4] = dSquareStatic.d;

    outputBuffer[5] = fwd_diff(Outer<float>.Inner<int>.cubeStatic<int>)(diffPair(2.0, 1.0), 0).d;

    var dCubeStatic = diffPair(2.0);
    bwd_diff(Outer<float>.Inner<int>.cubeStatic<int>)(dCubeStatic, 0, 1.0);
    outputBuffer[6] = dCubeStatic.d;

    var dA = diffPair(4.0);
    bwd_diff(L1<float>.L2<float>.L3<float>.squareA<int>)(dA, 0, 1.0);
    outputBuffer[7] = dA.d;

    var dB = diffPair(5.0);
    bwd_diff(L1<float>.L2<float>.L3<float>.squareB<int>)(dB, 0, 1.0);
    outputBuffer[8] = dB.d;

    var dC = diffPair(6.0);
    bwd_diff(L1<float>.L2<float>.L3<float>.squareC<int>)(dC, 0, 1.0);
    outputBuffer[9] = dC.d;

    var dIdentity = diffPair(1.5);
    bwd_diff(Wrapper<float>.Inner<int>.identity<int>)(dIdentity, 0, 1.0);
    outputBuffer[10] = dIdentity.d;

    var dCustom = diffPair(1.5);
    bwd_diff(Wrapper<float>.Inner<int>.identityWithCustomBwd<int>)(dCustom, 0, 1.0);
    outputBuffer[11] = dCustom.d;

    outputBuffer[12] = fwd_diff(Outer<float>.Inner<int>.cubeViaPrivate<int>)(diffPair(2.0, 1.0), 0).d;

    var dViaPrivate = diffPair(3.0);
    bwd_diff(Outer<float>.Inner<int>.cubeViaPrivate<int>)(dViaPrivate, 0, 1.0);
    outputBuffer[13] = dViaPrivate.d;

    // CHECK:      6.0
    // CHECK-NEXT: 12.0
    // CHECK-NEXT: 147.0
    // CHECK-NEXT: 6.0
    // CHECK-NEXT: 6.0
    // CHECK-NEXT: 12.0
    // CHECK-NEXT: 12.0
    // CHECK-NEXT: 8.0
    // CHECK-NEXT: 10.0
    // CHECK-NEXT: 12.0
    // CHECK-NEXT: 1.0
    // CHECK-NEXT: 2.0
    // CHECK-NEXT: 12.0
    // CHECK-NEXT: 27.0
}
