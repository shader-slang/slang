// Test FFLayer backward pass via autodiff.
// Verifies that autodiff correctly computes gradients through eval<A>().
//
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DTEST_POINTER=0 -xslang -DTEST_TENSORVIEW=0
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -xslang -experimental-feature -xslang -DTEST_POINTER=0 -xslang -DTEST_TENSORVIEW=0
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -xslang -experimental-feature -xslang -DTEST_POINTER=1 -xslang -DTEST_TENSORVIEW=0
//
// Compile-only test for TorchTensorViewAddress (CUDA target):
//TEST:SIMPLE(filecheck=CHECK): -target ptx -stage compute -entry computeMain -capability cuda_sm_7_0 -DTEST_TENSORVIEW=1 -DTEST_POINTER=0 -experimental-feature
// CHECK: computeMain

import slang.neural;

// Weights: W = [[1,2],[3,4]], bias = [0.5, -0.5]
//TEST_INPUT: set params = ubuffer(data=[1.0 2.0 3.0 4.0 0.5 -0.5], stride=4)

// Gradient buffer for weight gradients
//TEST_INPUT: set gradParams = ubuffer(data=[0.0 0.0 0.0 0.0 0.0 0.0], stride=4)

#if TEST_TENSORVIEW
    uniform TensorView<float> params;
    uniform TensorView<float> gradParams;
    typealias Address = TorchTensorViewAddress<float>;
#elif TEST_POINTER
    uniform float* params;
    uniform float* gradParams;
    typealias Address = PointerAddress<float>;
#else
    uniform RWStructuredBuffer<float>.Handle params;
    uniform RWStructuredBuffer<float>.Handle gradParams;
    typealias Address = BindlessAddress<float>;
#endif

//TEST_INPUT: ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=testResult
RWStructuredBuffer<uint> testResult;

typealias Vec2 = InlineVector<float, 2>;
typealias Act = IdentityActivation<float>;
typealias Layer = FFLayer<float, Vec2, Vec2, LinearLayout, Act, true>;

bool approxEqual(float a, float b, float eps = 0.001)
{
    return abs(a - b) < eps;
}

// Test forward pass
void testForwardPass(inout uint idx)
{
    let baseAddr = Address(params);
    let layer = Layer();

    float[2] xArr = {1.0, 2.0};
    let x = Vec2(xArr);
    let y = layer.eval<Address>(x, baseAddr);

    // W = [[1,2],[3,4]], b = [0.5, -0.5]
    // y = W*x + b = [1*1+2*2, 3*1+4*2] + [0.5, -0.5] = [5, 11] + [0.5, -0.5] = [5.5, 10.5]
    testResult[idx++] = approxEqual(y[0], 5.5);   // y[0] == 5.5
    testResult[idx++] = approxEqual(y[1], 10.5);   // y[1] == 10.5
}

// Wrapper function for autodiff - eval internally calls getOffset to derive
// the bias address, exercising the custom derivative of getOffset through autodiff.
[Differentiable]
Vec2 computeOutput(Address baseAddr, Vec2 input, Layer layer)
{
    return layer.eval<Address>(input, baseAddr);
}

// Test backward pass using autodiff
void testBackwardPass(inout uint idx)
{
    // Reset gradient buffer
    for (int i = 0; i < 6; i++)
        gradParams[i] = 0.0;

    let baseAddr = Address(params);
    let gradBaseAddr = Address(gradParams);
    let layer = Layer();

    float[2] xArr = {1.0, 2.0};
    let x = Vec2(xArr);

    // Create DifferentialPtrPair for the single base address (primal, gradient)
    var baseAddrPair = DifferentialPtrPair<Address>(baseAddr, gradBaseAddr);
    
    // Create diffPair for input
    var inputPair = diffPair(x);
    
    // Output gradient: dL/dy = [1, 1]
    let dOutput = Vec2(1.0);

    // Use bwd_diff to compute gradients - getOffset is called inside computeOutput,
    // and its custom derivative propagates the DifferentialPtrPair correctly.
    bwd_diff(computeOutput)(baseAddrPair, inputPair, layer, dOutput);

    // Expected weight gradients:
    // dL/dW = dy * x^T where dy = [1, 1], x = [1, 2]
    // dW[0,0] = dy[0] * x[0] = 1 * 1 = 1
    // dW[0,1] = dy[0] * x[1] = 1 * 2 = 2
    // dW[1,0] = dy[1] * x[0] = 1 * 1 = 1
    // dW[1,1] = dy[1] * x[1] = 1 * 2 = 2
    // dB[0] = dy[0] = 1
    // dB[1] = dy[1] = 1

    testResult[idx++] = approxEqual(gradParams[0], 1.0);  // dW[0,0]
    testResult[idx++] = approxEqual(gradParams[1], 2.0);  // dW[0,1]
    testResult[idx++] = approxEqual(gradParams[2], 1.0);  // dW[1,0]
    testResult[idx++] = approxEqual(gradParams[3], 2.0);  // dW[1,1]
    testResult[idx++] = approxEqual(gradParams[4], 1.0);  // dB[0]
    testResult[idx++] = approxEqual(gradParams[5], 1.0);  // dB[1]

}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    // Buffer layout:
    // [0,1]: fwd y[0]==5.5, y[1]==10.5
    // [2]: fwd combined
    // [3..8]: dW[0,0], dW[0,1], dW[1,0], dW[1,1], dB[0], dB[1]
    // [9]: bwd combined
    uint idx = 0;
    testForwardPass(idx);
    testBackwardPass(idx);
    // BUFFER: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
    // BUFFER-NEXT: 1
}
