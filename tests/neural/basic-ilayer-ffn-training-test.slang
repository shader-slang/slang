// Unit test for ILayer with multi-layer FFN (forward pass only).
//
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-dx12 -compute -shaderobj -profile cs_6_6 -xslang -experimental-feature -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-mtl -compute -shaderobj -output-using-type -xslang -experimental-feature
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0 -xslang -experimental-feature

import neural;

// Parameter packing:
// Layer1: FFLayer<float, InlineVector<float,2>, InlineVector<float,2>, IdentityActivation<float>, HasBias=true>
//   weights (2x2): [ 2, -1;
//                   0.5, 3 ]
//   bias (2): [ 1, -2 ]
// Layer2: FFLayer<float, InlineVector<float,2>, InlineVector<float,1>, IdentityActivation<float>, HasBias=true>
//   weights (1x2): [ -2, 4 ]
//   bias (1): [ 0.5 ]
//
// Total params: (2*2 + 2) + (1*2 + 1) = 9

//TEST_INPUT: set parametersFloat = ubuffer(data=[2.0 -1.0 0.5 3.0  1.0 -2.0  -2.0 4.0  0.5], stride=4)
RWStructuredBuffer<float> parametersFloat;

//TEST_INPUT: set params = ubuffer(data=[0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0], stride=4)
uniform RWStructuredBuffer<float>.Handle params;

//TEST_INPUT: ubuffer(data=[0 0], stride=4):out,name=resultBuffer
RWStructuredBuffer<uint> resultBuffer;

typealias Address = BindlessAddress<float>;
typealias V2 = InlineVector<float, 2>;
typealias V1 = InlineVector<float, 1>;
typealias Act = IdentityActivation<float>;
typealias Layer1 = FFLayer<float, V2, V2, Address, LinearLayout, Act, true>;
typealias Layer2 = FFLayer<float, V2, V1, Address, LinearLayout, Act, true>;

static const int Base0 = 0;

bool forwardCheck()
{
    let baseAddr = Address(params);

    float[2] xArr = { 1.5, -2.0 };
    let x = V2(xArr);

    // Layer1: weights at 0 (4 floats: 2x2), bias at 4 (2 floats)
    let layer1 = Layer1(int(0), int(4));
    // Layer2: weights at 6 (2 floats: 1x2), bias at 8 (1 float)
    let layer2 = Layer2(int(6), int(8));

    // Compute weight/bias addresses for each layer
    let w1Addr = baseAddr.getOffset(0);
    let b1Addr = baseAddr.getOffset(4);
    let w2Addr = baseAddr.getOffset(6);
    let b2Addr = baseAddr.getOffset(8);

    let h = layer1.eval<Address>(w1Addr, b1Addr, x);
    let y = layer2.eval<Address>(w2Addr, b2Addr, h);

    // Expected:
    // h = W1*x + b1
    //   = [2*1.5 + (-1)*(-2), 0.5*1.5 + 3*(-2)] + [1, -2]
    //   = [3 + 2, 0.75 - 6] + [1, -2]
    //   = [5 + 1, -5.25 - 2]
    //   = [6, -7.25]
    //
    // y = W2*h + b2
    //   = [-2*6 + 4*(-7.25)] + 0.5
    //   = [-12 - 29] + 0.5
    //   = -41 + 0.5
    //   = -40.5
    return abs(y[0] - (-40.5)) < 0.001;
}

bool parameterCountCheck()
{
    // Layer1: 2*2 + 2 = 6 parameters
    // Layer2: 1*2 + 1 = 3 parameters
    // Total: 9
    bool layer1Ok = (Layer1.ParameterCount == 6);
    bool layer2Ok = (Layer2.ParameterCount == 3);
    bool addressOk = (Layer1.nextOffset(Base0) == 6);
    return layer1Ok && layer2Ok && addressOk;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    // Copy parametersFloat -> params
    for (int i = 0; i < 9; i++)
        params[i] = parametersFloat[i];

    resultBuffer[0] = forwardCheck();
    resultBuffer[1] = parameterCountCheck();

    // BUFFER: 1
    // BUFFER-NEXT: 1
}
