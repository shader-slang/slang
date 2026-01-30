// Comprehensive test for all activation functions in the neural module.
// This test uses InlineVector. For CooperativeMatrix accelerated WaveTangledVector tests,
// see activation-coopmat-vector-test.slang.
//
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-dx12 -compute -shaderobj -profile cs_6_6 -xslang -experimental-feature -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-mtl -compute -shaderobj -output-using-type -xslang -experimental-feature
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0 -xslang -experimental-feature

import neural;

//TEST_INPUT: ubuffer(data=[0 0 0 0 0 0 0 0 0], stride=4):out,name=testResult
RWStructuredBuffer<uint> testResult;

typealias Vec3 = InlineVector<float, 3>;

// Helper: check if two floats are approximately equal
bool approxEqual(float a, float b, float eps = 0.0001)
{
    return abs(a - b) < eps;
}

bool approxEqualVec3(Vec3 a, Vec3 b, float eps = 0.0001)
{
    return approxEqual(a[0], b[0], eps) &&
           approxEqual(a[1], b[1], eps) &&
           approxEqual(a[2], b[2], eps);
}

// Test 1: IdentityActivation - returns input unchanged
bool testIdentity()
{
    float[3] arr = {-1.0, 0.0, 2.5};
    let input = Vec3(arr);
    
    let output = IdentityActivation<float>().eval<Vec3>(input);
    
    // Expected: same as input
    return approxEqualVec3(output, input);
}

// Test 2: ReLU - max(x, 0)
bool testReLU()
{
    float[3] arr = {-2.0, 0.0, 3.0};
    let input = Vec3(arr);
    
    let output = ReLU<float>().eval<Vec3>(input);
    
    // Expected: [0, 0, 3]
    float[3] expectedArr = {0.0, 0.0, 3.0};
    let expected = Vec3(expectedArr);
    return approxEqualVec3(output, expected);
}

// Test 3: LeakyReLU - x < 0 ? alpha*x : x
bool testLeakyReLU()
{
    float[3] arr = {-2.0, 0.0, 3.0};
    let input = Vec3(arr);
    float alpha = 0.1;
    
    let output = LeakyReLU<float>(alpha).eval<Vec3>(input);
    
    // Expected: [-0.2, 0, 3]
    float[3] expectedArr = {-0.2, 0.0, 3.0};
    let expected = Vec3(expectedArr);
    return approxEqualVec3(output, expected);
}

// Test 4: Sigmoid - 1 / (1 + exp(-x))
bool testSigmoid()
{
    float[3] arr = {0.0, 1.0, -1.0};
    let input = Vec3(arr);
    
    let output = Sigmoid<float>().eval<Vec3>(input);
    
    // Expected: sigmoid(0)=0.5, sigmoid(1)≈0.7311, sigmoid(-1)≈0.2689
    float[3] expectedArr = {0.5, 0.7310586, 0.2689414};
    let expected = Vec3(expectedArr);
    return approxEqualVec3(output, expected, 0.001);
}

// Test 5: TanhActivation - tanh(x)
bool testTanh()
{
    float[3] arr = {0.0, 1.0, -1.0};
    let input = Vec3(arr);
    
    let output = TanhActivation<float>().eval<Vec3>(input);
    
    // Expected: tanh(0)=0, tanh(1)≈0.7616, tanh(-1)≈-0.7616
    float[3] expectedArr = {0.0, 0.7615942, -0.7615942};
    let expected = Vec3(expectedArr);
    return approxEqualVec3(output, expected, 0.001);
}

// Test 6: ExpActivation - exp(x)
bool testExp()
{
    float[3] arr = {0.0, 1.0, -1.0};
    let input = Vec3(arr);
    
    let output = ExpActivation<float>().eval<Vec3>(input);
    
    // Expected: exp(0)=1, exp(1)≈2.7183, exp(-1)≈0.3679
    float[3] expectedArr = {1.0, 2.7182818, 0.3678794};
    let expected = Vec3(expectedArr);
    return approxEqualVec3(output, expected, 0.001);
}

// Test 7: SineActivation - sin(x)
bool testSine()
{
    float pi = 3.14159265;
    float[3] arr = {0.0, pi / 2.0, pi};
    let input = Vec3(arr);
    
    let output = SineActivation<float>().eval<Vec3>(input);
    
    // Expected: sin(0)=0, sin(pi/2)=1, sin(pi)≈0
    float[3] expectedArr = {0.0, 1.0, 0.0};
    let expected = Vec3(expectedArr);
    return approxEqualVec3(output, expected, 0.001);
}

// Test 8: SiLU (Swish) - x * sigmoid(x)
bool testSiLU()
{
    float[3] arr = {0.0, 1.0, -1.0};
    let input = Vec3(arr);
    
    let output = SiLU<float>().eval<Vec3>(input);
    
    // Expected: SiLU(0)=0, SiLU(1)=1*sigmoid(1)≈0.7311, SiLU(-1)=-1*sigmoid(-1)≈-0.2689
    float[3] expectedArr = {0.0, 0.7310586, -0.2689414};
    let expected = Vec3(expectedArr);
    return approxEqualVec3(output, expected, 0.001);
}

// Test 9: QuickGELU - x * sigmoid(1.702 * x)
bool testQuickGELU()
{
    float[3] arr = {0.0, 1.0, -1.0};
    let input = Vec3(arr);
    
    let output = QuickGELU<float>().eval<Vec3>(input);
    
    // Expected: QuickGELU(0)=0, QuickGELU(1)=1*sigmoid(1.702)≈0.8458, QuickGELU(-1)=-1*sigmoid(-1.702)≈-0.1542
    float[3] expectedArr = {0.0, 0.8458, -0.1542};
    let expected = Vec3(expectedArr);
    return approxEqualVec3(output, expected, 0.001);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    testResult[0] = testIdentity();
    testResult[1] = testReLU();
    testResult[2] = testLeakyReLU();
    testResult[3] = testSigmoid();
    testResult[4] = testTanh();
    testResult[5] = testExp();
    testResult[6] = testSine();
    testResult[7] = testSiLU();
    testResult[8] = testQuickGELU();

    // BUFFER: 1
    // BUFFER: 1
    // BUFFER: 1
    // BUFFER: 1
    // BUFFER: 1
    // BUFFER: 1
    // BUFFER: 1
    // BUFFER: 1
    // BUFFER: 1
}
