// Test FFLayer backward pass via autodiff.
// Verifies that autodiff correctly computes gradients through eval<A>().
//
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -xslang -DTEST_POINTER=0
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -xslang -experimental-feature -xslang -DTEST_POINTER=0
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -xslang -experimental-feature -xslang -DTEST_POINTER=1

import neural;

// Weights: W = [[1,2],[3,4]], bias = [0.5, -0.5]
//TEST_INPUT: set params = ubuffer(data=[1.0 2.0 3.0 4.0 0.5 -0.5], stride=4)

// Gradient buffer for weight gradients
//TEST_INPUT: set gradParams = ubuffer(data=[0.0 0.0 0.0 0.0 0.0 0.0], stride=4)

#if TEST_POINTER
    uniform float* params;
    uniform float* gradParams;
    typealias Address = PointerAddress<float>;
#else
    uniform RWStructuredBuffer<float>.Handle params;
    uniform RWStructuredBuffer<float>.Handle gradParams;
    typealias Address = BindlessAddress<float>;
#endif

//TEST_INPUT: ubuffer(data=[0 0], stride=4):out,name=testResult
RWStructuredBuffer<uint> testResult;

typealias Vec2 = InlineVector<float, 2>;
typealias Act = IdentityActivation<float>;
typealias Layer = FFLayer<float, Vec2, Vec2, Address, LinearLayout, Act, true>;

bool approxEqual(float a, float b, float eps = 0.001)
{
    return abs(a - b) < eps;
}

// Test forward pass
bool testForwardPass()
{
    let baseAddr = Address(params);
    let layer = Layer(0, 4);

    let weightAddr = baseAddr.getOffset(0);
    let biasAddr = baseAddr.getOffset(4);

    float[2] xArr = {1.0, 2.0};
    let x = Vec2(xArr);
    let y = layer.eval<Address>(weightAddr, biasAddr, x);

    // W = [[1,2],[3,4]], b = [0.5, -0.5]
    // y = W*x + b = [1*1+2*2, 3*1+4*2] + [0.5, -0.5] = [5, 11] + [0.5, -0.5] = [5.5, 10.5]
    return approxEqual(y[0], 5.5) && approxEqual(y[1], 10.5);
}

// Wrapper function for autodiff - takes DifferentialPtrPair for weight and bias addresses
[Differentiable]
Vec2 computeOutput(Address weightAddr, Address biasAddr, Vec2 input, Layer layer)
{
    return layer.eval<Address>(weightAddr, biasAddr, input);
}

// Test backward pass using autodiff
bool testBackwardPass()
{
    // Reset gradient buffer
    for (int i = 0; i < 6; i++)
        gradParams[i] = 0.0;

    let baseAddr = Address(params);
    let gradBaseAddr = Address(gradParams);
    let layer = Layer(0, 4);

    // Compute weight/bias addresses outside the differentiable function
    let weightAddr = baseAddr.getOffset(0);
    let biasAddr = baseAddr.getOffset(4);
    let dWeightAddr = gradBaseAddr.getOffset(0);
    let dBiasAddr = gradBaseAddr.getOffset(4);

    float[2] xArr = {1.0, 2.0};
    let x = Vec2(xArr);

    // Create DifferentialPtrPair for weight and bias addresses (primal, gradient)
    var weightAddrPair = DifferentialPtrPair<Address>(weightAddr, dWeightAddr);
    var biasAddrPair = DifferentialPtrPair<Address>(biasAddr, dBiasAddr);
    
    // Create diffPair for input
    var inputPair = diffPair(x);
    
    // Output gradient: dL/dy = [1, 1]
    let dOutput = Vec2(1.0);

    // Use bwd_diff to compute gradients
    bwd_diff(computeOutput)(weightAddrPair, biasAddrPair, inputPair, layer, dOutput);

    // Expected weight gradients:
    // dL/dW = dy * x^T where dy = [1, 1], x = [1, 2]
    // dW[0,0] = dy[0] * x[0] = 1 * 1 = 1
    // dW[0,1] = dy[0] * x[1] = 1 * 2 = 2
    // dW[1,0] = dy[1] * x[0] = 1 * 1 = 1
    // dW[1,1] = dy[1] * x[1] = 1 * 2 = 2
    // dB[0] = dy[0] = 1
    // dB[1] = dy[1] = 1

    bool w00 = approxEqual(gradParams[0], 1.0);
    bool w01 = approxEqual(gradParams[1], 2.0);
    bool w10 = approxEqual(gradParams[2], 1.0);
    bool w11 = approxEqual(gradParams[3], 2.0);
    bool b0 = approxEqual(gradParams[4], 1.0);
    bool b1 = approxEqual(gradParams[5], 1.0);

    return w00 && w01 && w10 && w11 && b0 && b1;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    testResult[0] = testForwardPass();
    testResult[1] = testBackwardPass();
    // BUFFER: 1
    // BUFFER: 1
}
