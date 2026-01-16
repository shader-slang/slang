// Basic test for builtin neural "frontend" APIs (layers + activations).
//
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-dx12 -compute -shaderobj -profile cs_6_6 -xslang -experimental-feature -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-mtl -compute -shaderobj -output-using-type -xslang -experimental-feature
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0 -xslang -experimental-feature

import neural;

// Parameters: FFLayer<float, 4, 2, ..., HasBias=true>
// weights = 2x4 matrix, bias = 2
//TEST_INPUT: ubuffer(data=[1 2 3 4  5 6 7 8  9 10], stride=4):name=parametersFloat
RWStructuredBuffer<float> parametersFloat;

//TEST_INPUT: ubuffer(data=[0 0 0 0 0 0 0 0 0 0], stride=4):name=params
RWStructuredBuffer<float> params;

//TEST_INPUT: ubuffer(data=[0 0], stride=4):out,name=testResult
RWStructuredBuffer<uint> testResult;

typealias Storage = StructuredBufferStorage<float>;
typealias XVec = InlineVector<float, 4>;
typealias YVec = InlineVector<float, 2>;
typealias Act = ReLU<float>;
typealias Layer = FFLayer<float, XVec, YVec, Storage, Act, true>;

// Test: evaluate layer through ILayer constraint
InlineVector<float, 2> evalAsLayer<L>(
    L layer,
    Act.ParamType activationParam,
    InlineVector<float, 4> input)
    where L : ILayer<float, InlineVector<float, 4>, InlineVector<float, 2>, Storage, Act>
{
    return layer.eval(activationParam, input);
}

bool forwardTest()
{
    // Copy parametersFloat -> params
    for(int i=0;i<10;i++) params[i] = parametersFloat[i];

    let storage = Storage(params);
    // Weights at offset 0 (8 floats: 2x4), bias at offset 8 (2 floats)
    let layer = Layer(storage, 0, 8);

    // Input x = [1,2,3,4]
    float[4] xArr = {1.0, 2.0, 3.0, 4.0};
    let x = InlineVector<float, 4>(xArr);

    // Layer: y = ReLU(W*x + b)
    let y = evalAsLayer<Layer>(layer, NoParam(), x);

    // Expected: W*x + b = [1*1+2*2+3*3+4*4 + 9, 5*1+6*2+7*3+8*4 + 10] = [39, 80]
    // ReLU([39, 80]) = [39, 80]
    return (y[0] == 39.0) && (y[1] == 80.0);
}

bool parameterCountTest()
{
    // FFLayer<float, 4->2, HasBias=true> should have 4*2 + 2 = 10 parameters
    return Layer.ParameterCount == 10;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    testResult[0] = forwardTest();
    testResult[1] = parameterCountTest();

    // BUFFER: 1
    // BUFFER: 1
}
