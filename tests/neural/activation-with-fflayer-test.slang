// Test activations integrated with FFLayer (layer + activation end-to-end).
//
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-dx12 -compute -shaderobj -profile cs_6_6 -xslang -experimental-feature -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-mtl -compute -shaderobj -output-using-type -xslang -experimental-feature
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0 -xslang -experimental-feature

import neural;

// Simple weights for testing: identity-like transform
// For 2x2 -> 2: W = [[1,0],[0,1]], b = [0,0]
// This makes W*x + b = x (for 2D input/output)
//TEST_INPUT: ubuffer(data=[1.0 0.0 0.0 1.0 0.0 0.0], stride=4):name=identityParams
RWStructuredBuffer<float> identityParams;

// Params buffer (will be copied from identityParams)
//TEST_INPUT: ubuffer(data=[0.0 0.0 0.0 0.0 0.0 0.0], stride=4):name=params
RWStructuredBuffer<float> params;

//TEST_INPUT: ubuffer(data=[0 0 0 0 0 0 0], stride=4):out,name=testResult
RWStructuredBuffer<uint> testResult;

typealias Storage = StructuredBufferStorage<float>;
typealias Vec2 = InlineVector<float, 2>;

// Helper: check if two floats are approximately equal
bool approxEqual(float a, float b, float eps = 0.0001)
{
    return abs(a - b) < eps;
}

void setupParams()
{
    for(int i = 0; i < 6; i++)
        params[i] = identityParams[i];
}

// Test ReLU with FFLayer
// Input: [-1, 2] -> W*x+b = [-1, 2] -> ReLU = [0, 2]
bool testFFLayerReLU()
{
    setupParams();
    let storage = Storage(params);
    
    typealias Layer = FFLayer<float, Vec2, Vec2, Storage, LinearLayout, ReLU<float>, true>;
    let layer = Layer(0, 4);  // weights at 0, bias at 4
    
    float[2] arr = {-1.0, 2.0};
    let x = Vec2(arr);
    
    let y = layer.eval<Storage>(storage, x);
    
    // ReLU([-1, 2]) = [0, 2]
    return approxEqual(y[0], 0.0) && approxEqual(y[1], 2.0);
}

// Test LeakyReLU with FFLayer
// Input: [-1, 2] -> W*x+b = [-1, 2] -> LeakyReLU(alpha=0.1) = [-0.1, 2]
bool testFFLayerLeakyReLU()
{
    setupParams();
    let storage = Storage(params);
    
    typealias Layer = FFLayer<float, Vec2, Vec2, Storage, LinearLayout, LeakyReLU<float>, true>;
    let leakyRelu = LeakyReLU<float>(0.1);  // alpha = 0.1
    let layer = Layer(0, 4, leakyRelu);
    
    float[2] arr = {-1.0, 2.0};
    let x = Vec2(arr);
    
    let y = layer.eval<Storage>(storage, x);
    
    // LeakyReLU([-1, 2], 0.1) = [-0.1, 2]
    return approxEqual(y[0], -0.1) && approxEqual(y[1], 2.0);
}

// Test Sigmoid with FFLayer
// Input: [0, 0] -> W*x+b = [0, 0] -> Sigmoid = [0.5, 0.5]
bool testFFLayerSigmoid()
{
    setupParams();
    let storage = Storage(params);
    
    typealias Layer = FFLayer<float, Vec2, Vec2, Storage, LinearLayout, Sigmoid<float>, true>;
    let layer = Layer(0, 4);
    
    float[2] arr = {0.0, 0.0};
    let x = Vec2(arr);
    
    let y = layer.eval<Storage>(storage, x);
    
    // Sigmoid([0, 0]) = [0.5, 0.5]
    return approxEqual(y[0], 0.5) && approxEqual(y[1], 0.5);
}

// Test TanhActivation with FFLayer
// Input: [0, 0] -> W*x+b = [0, 0] -> Tanh = [0, 0]
bool testFFLayerTanh()
{
    setupParams();
    let storage = Storage(params);
    
    typealias Layer = FFLayer<float, Vec2, Vec2, Storage, LinearLayout, TanhActivation<float>, true>;
    let layer = Layer(0, 4);
    
    float[2] arr = {0.0, 0.0};
    let x = Vec2(arr);
    
    let y = layer.eval<Storage>(storage, x);
    
    // Tanh([0, 0]) = [0, 0]
    return approxEqual(y[0], 0.0) && approxEqual(y[1], 0.0);
}

// Test ExpActivation with FFLayer
// Input: [0, 0] -> W*x+b = [0, 0] -> Exp = [1, 1]
bool testFFLayerExp()
{
    setupParams();
    let storage = Storage(params);
    
    typealias Layer = FFLayer<float, Vec2, Vec2, Storage, LinearLayout, ExpActivation<float>, true>;
    let layer = Layer(0, 4);
    
    float[2] arr = {0.0, 0.0};
    let x = Vec2(arr);
    
    let y = layer.eval<Storage>(storage, x);
    
    // Exp([0, 0]) = [1, 1]
    return approxEqual(y[0], 1.0) && approxEqual(y[1], 1.0);
}

// Test IdentityActivation with FFLayer
// Input: [-1, 2] -> W*x+b = [-1, 2] -> Identity = [-1, 2]
bool testFFLayerIdentity()
{
    setupParams();
    let storage = Storage(params);
    
    typealias Layer = FFLayer<float, Vec2, Vec2, Storage, LinearLayout, IdentityActivation<float>, true>;
    let layer = Layer(0, 4);
    
    float[2] arr = {-1.0, 2.0};
    let x = Vec2(arr);
    
    let y = layer.eval<Storage>(storage, x);
    
    // Identity([-1, 2]) = [-1, 2]
    return approxEqual(y[0], -1.0) && approxEqual(y[1], 2.0);
}

// Test SineActivation with FFLayer
// Input: [0, pi/2] -> W*x+b = [0, pi/2] -> Sin = [0, 1]
bool testFFLayerSine()
{
    setupParams();
    let storage = Storage(params);
    
    typealias Layer = FFLayer<float, Vec2, Vec2, Storage, LinearLayout, SineActivation<float>, true>;
    let layer = Layer(0, 4);
    
    float pi = 3.14159265;
    float[2] arr = {0.0, pi / 2.0};
    let x = Vec2(arr);
    
    let y = layer.eval<Storage>(storage, x);
    
    // Sin([0, pi/2]) = [0, 1]
    return approxEqual(y[0], 0.0, 0.001) && approxEqual(y[1], 1.0, 0.001);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    testResult[0] = testFFLayerReLU();
    testResult[1] = testFFLayerLeakyReLU();
    testResult[2] = testFFLayerSigmoid();
    testResult[3] = testFFLayerTanh();
    testResult[4] = testFFLayerExp();
    testResult[5] = testFFLayerIdentity();
    testResult[6] = testFFLayerSine();

    // BUFFER: 1
    // BUFFER: 1
    // BUFFER: 1
    // BUFFER: 1
    // BUFFER: 1
    // BUFFER: 1
    // BUFFER: 1
}
