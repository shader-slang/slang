// Test FFLayer with HasBias=false (forward and backward).
// Verifies that y = Activation(W*x) works correctly without bias,
// and that autodiff computes correct weight and input gradients.
//
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_POINTER=0
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -xslang -experimental-feature -xslang -DTEST_POINTER=0
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -xslang -experimental-feature -xslang -DTEST_POINTER=1

import neural;

// Weights only (no bias): W = [[1,2],[3,4]]
//TEST_INPUT: set params = ubuffer(data=[1.0 2.0 3.0 4.0], stride=4)

// Gradient buffer for weight gradients
//TEST_INPUT: set gradParams = ubuffer(data=[0.0 0.0 0.0 0.0], stride=4)

#if TEST_POINTER
    uniform float* params;
    uniform float* gradParams;
    typealias Address = PointerAddress<float>;
#else
    uniform RWStructuredBuffer<float>.Handle params;
    uniform RWStructuredBuffer<float>.Handle gradParams;
    typealias Address = BindlessAddress<float>;
#endif

//TEST_INPUT: ubuffer(data=[0 0 0], stride=4):out,name=testResult
RWStructuredBuffer<uint> testResult;

typealias Vec2 = InlineVector<float, 2>;
typealias Act = IdentityActivation<float>;
typealias Layer = FFLayer<float, Vec2, Vec2, LinearLayout, Act, false>;

bool approxEqual(float a, float b, float eps = 0.001)
{
    return abs(a - b) < eps;
}

// Test forward pass: y = W*x (no bias)
bool testForwardPass()
{
    let baseAddr = Address(params);
    let layer = Layer();

    let weightAddr = baseAddr.getOffset(0);

    float[2] xArr = {1.0, 2.0};
    let x = Vec2(xArr);

    // Call eval without bias address (uses default none)
    let y = layer.eval<Address>(x, weightAddr);

    // W = [[1,2],[3,4]]
    // y = W*x = [1*1+2*2, 3*1+4*2] = [5, 11]
    return approxEqual(y[0], 5.0) && approxEqual(y[1], 11.0);
}

// Test ParameterCount: should be 2*2 = 4 (no bias terms)
bool testParameterCount()
{
    return Layer.ParameterCount == 4;
}

// Wrapper function for autodiff (no bias address needed)
[Differentiable]
Vec2 computeOutput(Address weightAddr, Vec2 input, Layer layer)
{
    return layer.eval<Address>(input, weightAddr);
}

// Test backward pass using autodiff
bool testBackwardPass()
{
    // Reset gradient buffer
    for (int i = 0; i < 4; i++)
        gradParams[i] = 0.0;

    let baseAddr = Address(params);
    let gradBaseAddr = Address(gradParams);
    let layer = Layer();

    let weightAddr = baseAddr.getOffset(0);
    let dWeightAddr = gradBaseAddr.getOffset(0);

    float[2] xArr = {1.0, 2.0};
    let x = Vec2(xArr);

    // Create DifferentialPtrPair for weight address (primal, gradient)
    var weightAddrPair = DifferentialPtrPair<Address>(weightAddr, dWeightAddr);

    // Create diffPair for input
    var inputPair = diffPair(x);

    // Output gradient: dL/dy = [1, 1]
    let dOutput = Vec2(1.0);

    // Use bwd_diff to compute gradients
    bwd_diff(computeOutput)(weightAddrPair, inputPair, layer, dOutput);

    // Expected weight gradients:
    // dL/dW = dy * x^T where dy = [1, 1], x = [1, 2]
    // dW[0,0] = dy[0] * x[0] = 1 * 1 = 1
    // dW[0,1] = dy[0] * x[1] = 1 * 2 = 2
    // dW[1,0] = dy[1] * x[0] = 1 * 1 = 1
    // dW[1,1] = dy[1] * x[1] = 1 * 2 = 2
    bool w00 = approxEqual(gradParams[0], 1.0);
    bool w01 = approxEqual(gradParams[1], 2.0);
    bool w10 = approxEqual(gradParams[2], 1.0);
    bool w11 = approxEqual(gradParams[3], 2.0);

    // Expected input gradients:
    // dL/dx = W^T * dy = [[1,3],[2,4]] * [1,1] = [1+3, 2+4] = [4, 6]
    bool dx0 = approxEqual(inputPair.d[0], 4.0);
    bool dx1 = approxEqual(inputPair.d[1], 6.0);

    return w00 && w01 && w10 && w11 && dx0 && dx1;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    testResult[0] = testForwardPass();
    testResult[1] = testParameterCount();
    testResult[2] = testBackwardPass();
    // BUFFER: 1
    // BUFFER: 1
    // BUFFER: 1
}
