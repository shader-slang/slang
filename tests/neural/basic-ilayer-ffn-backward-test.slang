// Unit test for ILayer with multi-layer FFN (backward pass via autodiff).
// Exercises getOffset inside the differentiable function so the custom
// derivative of getOffset is used to propagate DifferentialPtrPair.
//
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-mtl -compute -shaderobj -output-using-type -xslang -experimental-feature
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0 -xslang -experimental-feature

import slang.neural;

// Same 2-layer FFN as the forward test:
//   Layer1 (2->2): W1 = [[2,-1],[0.5,3]], b1 = [1,-2]  (6 params at offset 0)
//   Layer2 (2->1): W2 = [[-2,4]],          b2 = [0.5]   (3 params at offset 6)
// Total: 9 parameters

//TEST_INPUT: set parametersFloat = ubuffer(data=[2.0 -1.0 0.5 3.0  1.0 -2.0  -2.0 4.0  0.5], stride=4)
RWStructuredBuffer<float> parametersFloat;

//TEST_INPUT: set params = ubuffer(data=[0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0], stride=4)
uniform RWStructuredBuffer<float>.Handle params;

//TEST_INPUT: set gradParams = ubuffer(data=[0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0], stride=4)
uniform RWStructuredBuffer<float>.Handle gradParams;

//TEST_INPUT: ubuffer(data=[0 0 0 0 0 0 0 0 0], stride=4):out,name=resultBuffer
RWStructuredBuffer<uint> resultBuffer;

typealias Address = BindlessAddress<float>;
typealias V2 = InlineVector<float, 2>;
typealias V1 = InlineVector<float, 1>;
typealias Act = IdentityActivation<float>;
typealias Layer1 = FFLayer<float, V2, V2, LinearLayout, Act, true>;
typealias Layer2 = FFLayer<float, V2, V1, LinearLayout, Act, true>;

bool approxEqual(float a, float b, float eps = 0.001)
{
    return abs(a - b) < eps;
}

// All address arithmetic happens inside the differentiable function,
// so getOffset's custom derivative propagates the DifferentialPtrPair.
[Differentiable]
V1 computeFFN(Address baseAddr, V2 input, Layer1 layer1, Layer2 layer2)
{
    let layer1Addr = baseAddr.getOffset(0);
    let layer2Addr = baseAddr.getOffset(6);

    let h = layer1.eval<Address>(input, layer1Addr);
    return layer2.eval<Address>(h, layer2Addr);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    for (int i = 0; i < 9; i++)
    {
        params[i] = parametersFloat[i];
        gradParams[i] = 0.0;
    }

    let baseAddr = Address(params);
    let gradBaseAddr = Address(gradParams);

    float[2] xArr = { 1.5, -2.0 };
    let x = V2(xArr);
    let layer1 = Layer1();
    let layer2 = Layer2();

    var baseAddrPair = DifferentialPtrPair<Address>(baseAddr, gradBaseAddr);
    var inputPair = diffPair(x);
    let dOutput = V1(1.0);

    bwd_diff(computeFFN)(baseAddrPair, inputPair, layer1, layer2, dOutput);

    // Expected gradients (dL/dy = [1]):
    //   Forward: h = W1*x + b1 = [6, -7.25], y = W2*h + b2 = -40.5
    //
    //   dL/dW1 = outer(W2^T * dL/dy, x) = outer([-2, 4], [1.5, -2])
    //     = [-3, 4, 6, -8]
    //   dL/db1 = W2^T * dL/dy = [-2, 4]
    //   dL/dW2 = outer(dL/dy, h) = [6, -7.25]
    //   dL/db2 = dL/dy = [1]
    uint idx = 0;
    resultBuffer[idx++] = approxEqual(gradParams[0], -3.0);    // dW1[0,0]
    resultBuffer[idx++] = approxEqual(gradParams[1],  4.0);    // dW1[0,1]
    resultBuffer[idx++] = approxEqual(gradParams[2],  6.0);    // dW1[1,0]
    resultBuffer[idx++] = approxEqual(gradParams[3], -8.0);    // dW1[1,1]
    resultBuffer[idx++] = approxEqual(gradParams[4], -2.0);    // db1[0]
    resultBuffer[idx++] = approxEqual(gradParams[5],  4.0);    // db1[1]
    resultBuffer[idx++] = approxEqual(gradParams[6],  6.0);    // dW2[0,0]
    resultBuffer[idx++] = approxEqual(gradParams[7], -7.25);   // dW2[0,1]
    resultBuffer[idx++] = approxEqual(gradParams[8],  1.0);    // db2

    // BUFFER: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
}
