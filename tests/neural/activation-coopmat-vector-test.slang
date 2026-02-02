// Test activation functions with CooperativeMatrix accelerated WaveTangledVector type.
// This test covers the same activation functions as activation-functions-test.slang
// but uses WaveTangledVector instead of InlineVector.
//
// Only run on CUDA and Vulkan (SPIR-V) which support cooperative matrix operations.
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0 -xslang -experimental-feature

import neural;

//TEST_INPUT: ubuffer(data=[0 0 0 0 0 0 0 0 0], stride=4):out,name=testResult
RWStructuredBuffer<uint> testResult;

typealias ElementType = float;

static const int VecSize = 3;
static const int SubgroupSize = 32;
static const int BatchSize = 32;

typealias ShMemSize = SharedMemorySize<ElementType, TargetEnum.CUDA, ExecutionMode.Inference, SubgroupSize, BatchSize / SubgroupSize>;
// Use a dummy layer size for shared memory calculation (we only need minimal shared memory for activations)
typealias ShMemSizeLayer = ShMemSize.OfLayer1<VecSize, VecSize>;
typealias ShMemPool = SharedMemoryPool<ShMemSizeLayer>;
typealias Vec3 = WaveTangledVector<ElementType, ShMemPool, VecSize, SubgroupSize>;

// Helper: check if two floats are approximately equal
bool approxEqual(ElementType a, ElementType b, ElementType eps = 0.0001)
{
    return abs(a - b) < eps;
}

bool approxEqualVec3(Vec3 a, Vec3 b, ElementType eps = 0.0001)
{
    return approxEqual(a[0], b[0], eps) &&
           approxEqual(a[1], b[1], eps) &&
           approxEqual(a[2], b[2], eps);
}

// Test 1: IdentityActivation - returns input unchanged
bool testIdentity()
{
    ElementType[VecSize] arr = {-1.0, 0.0, 2.5};
    let input = Vec3(arr);
    
    let output = IdentityActivation<ElementType>().eval<Vec3>(input);
    
    // Expected: same as input
    return approxEqualVec3(output, input);
}

// Test 2: ReLU - max(x, 0)
bool testReLU()
{
    ElementType[VecSize] arr = {-2.0, 0.0, 3.0};
    let input = Vec3(arr);
    
    let output = ReLU<ElementType>().eval<Vec3>(input);
    
    // Expected: [0, 0, 3]
    ElementType[VecSize] expectedArr = {0.0, 0.0, 3.0};
    let expected = Vec3(expectedArr);
    return approxEqualVec3(output, expected);
}

// Test 3: LeakyReLU - x < 0 ? alpha*x : x
bool testLeakyReLU()
{
    ElementType[VecSize] arr = {-2.0, 0.0, 3.0};
    let input = Vec3(arr);
    ElementType alpha = 0.1;
    
    let output = LeakyReLU<ElementType>(alpha).eval<Vec3>(input);
    
    // Expected: [-0.2, 0, 3]
    ElementType[VecSize] expectedArr = {-0.2, 0.0, 3.0};
    let expected = Vec3(expectedArr);
    return approxEqualVec3(output, expected);
}

// Test 4: Sigmoid - 1 / (1 + exp(-x))
bool testSigmoid()
{
    ElementType[VecSize] arr = {0.0, 1.0, -1.0};
    let input = Vec3(arr);
    
    let output = Sigmoid<ElementType>().eval<Vec3>(input);
    
    // Expected: sigmoid(0)=0.5, sigmoid(1)≈0.7311, sigmoid(-1)≈0.2689
    ElementType[VecSize] expectedArr = {0.5, 0.7310586, 0.2689414};
    let expected = Vec3(expectedArr);
    return approxEqualVec3(output, expected, 0.001);
}

// Test 5: TanhActivation - tanh(x)
bool testTanh()
{
    ElementType[VecSize] arr = {0.0, 1.0, -1.0};
    let input = Vec3(arr);
    
    let output = TanhActivation<ElementType>().eval<Vec3>(input);
    
    // Expected: tanh(0)=0, tanh(1)≈0.7616, tanh(-1)≈-0.7616
    ElementType[VecSize] expectedArr = {0.0, 0.7615942, -0.7615942};
    let expected = Vec3(expectedArr);
    return approxEqualVec3(output, expected, 0.001);
}

// Test 6: ExpActivation - exp(x)
bool testExp()
{
    ElementType[VecSize] arr = {0.0, 1.0, -1.0};
    let input = Vec3(arr);
    
    let output = ExpActivation<ElementType>().eval<Vec3>(input);
    
    // Expected: exp(0)=1, exp(1)≈2.7183, exp(-1)≈0.3679
    ElementType[VecSize] expectedArr = {1.0, 2.7182818, 0.3678794};
    let expected = Vec3(expectedArr);
    return approxEqualVec3(output, expected, 0.001);
}

// Test 7: SineActivation - sin(x)
bool testSine()
{
    ElementType pi = 3.14159265;
    ElementType[VecSize] arr = {0.0, pi / 2.0, pi};
    let input = Vec3(arr);
    
    let output = SineActivation<ElementType>().eval<Vec3>(input);
    
    // Expected: sin(0)=0, sin(pi/2)=1, sin(pi)≈0
    ElementType[VecSize] expectedArr = {0.0, 1.0, 0.0};
    let expected = Vec3(expectedArr);
    return approxEqualVec3(output, expected, 0.001);
}

// Test 8: SiLU (Swish) - x * sigmoid(x)
bool testSiLU()
{
    ElementType[VecSize] arr = {0.0, 1.0, -1.0};
    let input = Vec3(arr);
    
    let output = SiLU<ElementType>().eval<Vec3>(input);
    
    // Expected: SiLU(0)=0, SiLU(1)=1*sigmoid(1)≈0.7311, SiLU(-1)=-1*sigmoid(-1)≈-0.2689
    ElementType[VecSize] expectedArr = {0.0, 0.7310586, -0.2689414};
    let expected = Vec3(expectedArr);
    return approxEqualVec3(output, expected, 0.001);
}

// Test 9: QuickGELU - x * sigmoid(1.702 * x)
bool testQuickGELU()
{
    ElementType[VecSize] arr = {0.0, 1.0, -1.0};
    let input = Vec3(arr);
    
    let output = QuickGELU<ElementType>().eval<Vec3>(input);
    
    // Expected: QuickGELU(0)=0, QuickGELU(1)=1*sigmoid(1.702)≈0.8458, QuickGELU(-1)=-1*sigmoid(-1.702)≈-0.1542
    ElementType[VecSize] expectedArr = {0.0, 0.8458, -0.1542};
    let expected = Vec3(expectedArr);
    return approxEqualVec3(output, expected, 0.001);
}

[shader("compute")]
[numthreads(BatchSize, 1, 1)]
void computeMain(uint tid : SV_DispatchThreadID)
{
    // All threads compute the same result, but we only write from thread 0
    bool results[9];
    results[0] = testIdentity();
    results[1] = testReLU();
    results[2] = testLeakyReLU();
    results[3] = testSigmoid();
    results[4] = testTanh();
    results[5] = testExp();
    results[6] = testSine();
    results[7] = testSiLU();
    results[8] = testQuickGELU();

    // Ensure all threads in the wave agree on the results
    for (int i = 0; i < 9; i++)
    {
        results[i] = WaveActiveAllTrue(results[i]);
    }

    if (tid == 0)
    {
        testResult[0] = results[0];
        testResult[1] = results[1];
        testResult[2] = results[2];
        testResult[3] = results[3];
        testResult[4] = results[4];
        testResult[5] = results[5];
        testResult[6] = results[6];
        testResult[7] = results[7];
        testResult[8] = results[8];
    }

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
