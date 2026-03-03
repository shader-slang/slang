// Integration/stress test for a 4-layer MLP (3->128->128->128->3)
// using FFLayer with all activation functions.
// Tests forward pass correctness and backward pass (autodiff) gradient computation.
//
// This test verifies that the neural.slang module can handle a realistically-sized
// MLP with all supported activation functions, including both forward evaluation
// and backward gradient computation through the full network.
// The network outputs 3 channels (RGB) to match a real 256x256 reference image.
//
// Parameterized by ACTIVATION_TYPE:
//   0 = LeakyReLU, 1 = ReLU, 2 = Sigmoid, 3 = TanhActivation,
//   4 = ExpActivation, 5 = SineActivation, 6 = SiLU, 7 = QuickGELU,
//   8 = IdentityActivation
//
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DACTIVATION_TYPE=0
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DACTIVATION_TYPE=1
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DACTIVATION_TYPE=2
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DACTIVATION_TYPE=3
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DACTIVATION_TYPE=4
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DACTIVATION_TYPE=5
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DACTIVATION_TYPE=6
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DACTIVATION_TYPE=7
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -emit-spirv-directly -xslang -DACTIVATION_TYPE=8
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -xslang -experimental-feature -xslang -DACTIVATION_TYPE=0

import neural;

static const int INPUT_DIM = 3;
static const int HIDDEN_DIM = 128;
static const int OUTPUT_DIM = 3;

// Layer parameter counts
static const int L0_PARAMS = HIDDEN_DIM * INPUT_DIM + HIDDEN_DIM;       // 3*128 + 128 = 512
static const int L1_PARAMS = HIDDEN_DIM * HIDDEN_DIM + HIDDEN_DIM;     // 128*128 + 128 = 16512
static const int L2_PARAMS = HIDDEN_DIM * HIDDEN_DIM + HIDDEN_DIM;     // 128*128 + 128 = 16512
static const int L3_PARAMS = OUTPUT_DIM * HIDDEN_DIM + OUTPUT_DIM;     // 128*3 + 3 = 387
static const int TOTAL_PARAMS = L0_PARAMS + L1_PARAMS + L2_PARAMS + L3_PARAMS; // 33923

// Activation selection via preprocessor
#if ACTIVATION_TYPE == 0
typealias HiddenAct = LeakyReLU<float>;
#elif ACTIVATION_TYPE == 1
typealias HiddenAct = ReLU<float>;
#elif ACTIVATION_TYPE == 2
typealias HiddenAct = Sigmoid<float>;
#elif ACTIVATION_TYPE == 3
typealias HiddenAct = TanhActivation<float>;
#elif ACTIVATION_TYPE == 4
typealias HiddenAct = ExpActivation<float>;
#elif ACTIVATION_TYPE == 5
typealias HiddenAct = SineActivation<float>;
#elif ACTIVATION_TYPE == 6
typealias HiddenAct = SiLU<float>;
#elif ACTIVATION_TYPE == 7
typealias HiddenAct = QuickGELU<float>;
#elif ACTIVATION_TYPE == 8
typealias HiddenAct = IdentityActivation<float>;
#endif

typealias Address = BindlessAddress<float>;
typealias InputVec = InlineVector<float, INPUT_DIM>;
typealias HiddenVec = InlineVector<float, HIDDEN_DIM>;
typealias OutputVec = InlineVector<float, OUTPUT_DIM>;

typealias Layer0 = FFLayer<float, InputVec, HiddenVec, LinearLayout, HiddenAct, true>;
typealias Layer1 = FFLayer<float, HiddenVec, HiddenVec, LinearLayout, HiddenAct, true>;
typealias Layer2 = FFLayer<float, HiddenVec, HiddenVec, LinearLayout, HiddenAct, true>;
typealias Layer3 = FFLayer<float, HiddenVec, OutputVec, LinearLayout, IdentityActivation<float>, true>;

// Buffer declarations
//TEST_INPUT: set params = ubuffer(stride=4, count=33923)
uniform RWStructuredBuffer<float>.Handle params;

//TEST_INPUT: set gradParams = ubuffer(stride=4, count=33923)
uniform RWStructuredBuffer<float>.Handle gradParams;

//TEST_INPUT: ubuffer(data=[0 0 0 0 0], stride=4):out,name=testResult
RWStructuredBuffer<uint> testResult;

// Deterministic LCG random number generator
struct LCG
{
    uint state;
    __init(uint seed) { state = seed; }
    [mutating] uint nextUint() { state = 1664525u * state + 1013904223u; return state; }
    [mutating] float nextFloat() { return float(nextUint() >> 8) * 5.96046448e-8f; }
    [mutating] float nextGaussianApprox()
    {
        float sum = 0.0f;
        for (int i = 0; i < 6; i++) sum += nextFloat();
        return sum - 3.0f;
    }
};

// Xavier initialization for a layer's weights
void xavierInit(inout LCG rng, int fanIn, int fanOut, int offset)
{
    float scale = sqrt(6.0f / float(fanIn + fanOut));
    int weightCount = fanIn * fanOut;
    for (int i = 0; i < weightCount; i++)
        params[offset + i] = (rng.nextFloat() * 2.0f - 1.0f) * scale;
    for (int i = 0; i < fanOut; i++)
        params[offset + weightCount + i] = 0.0f;
}

void initializeWeights()
{
    LCG rng = LCG(42u);
    int offset = 0;
    xavierInit(rng, INPUT_DIM, HIDDEN_DIM, offset);
    offset += L0_PARAMS;
    xavierInit(rng, HIDDEN_DIM, HIDDEN_DIM, offset);
    offset += L1_PARAMS;
    xavierInit(rng, HIDDEN_DIM, HIDDEN_DIM, offset);
    offset += L2_PARAMS;
    xavierInit(rng, HIDDEN_DIM, OUTPUT_DIM, offset);
}

void clearGradients()
{
    for (int i = 0; i < TOTAL_PARAMS; i++)
        gradParams[i] = 0.0f;
}

#if ACTIVATION_TYPE == 0
HiddenAct makeHiddenActivation() { return LeakyReLU<float>(0.01f); }
#else
HiddenAct makeHiddenActivation() { return HiddenAct(); }
#endif

// Non-differentiable forward pass (for tests that don't need gradients)
OutputVec mlpForwardNoDiff(Address baseAddr, InputVec input)
{
    let act = makeHiddenActivation();
    int offset = 0;

    let w0 = baseAddr.getOffset(offset);
    let b0 = baseAddr.getOffset(offset + INPUT_DIM * HIDDEN_DIM);
    let h0 = Layer0(act).eval<Address>(input, w0, b0);
    offset += L0_PARAMS;

    let w1 = baseAddr.getOffset(offset);
    let b1 = baseAddr.getOffset(offset + HIDDEN_DIM * HIDDEN_DIM);
    let h1 = Layer1(act).eval<Address>(h0, w1, b1);
    offset += L1_PARAMS;

    let w2 = baseAddr.getOffset(offset);
    let b2 = baseAddr.getOffset(offset + HIDDEN_DIM * HIDDEN_DIM);
    let h2 = Layer2(act).eval<Address>(h1, w2, b2);
    offset += L2_PARAMS;

    let w3 = baseAddr.getOffset(offset);
    let b3 = baseAddr.getOffset(offset + HIDDEN_DIM * OUTPUT_DIM);
    return Layer3().eval<Address>(h2, w3, b3);
}

// Differentiable forward pass - addresses must be pre-computed and passed in
[Differentiable]
OutputVec mlpForwardDiff(
    Address w0, Address b0,
    Address w1, Address b1,
    Address w2, Address b2,
    Address w3, Address b3,
    InputVec input)
{
    let act = makeHiddenActivation();
    let h0 = Layer0(act).eval<Address>(input, w0, b0);
    let h1 = Layer1(act).eval<Address>(h0, w1, b1);
    let h2 = Layer2(act).eval<Address>(h1, w2, b2);
    return Layer3().eval<Address>(h2, w3, b3);
}

// Compute all layer addresses from a base address (non-differentiable)
void computeLayerAddresses(Address baseAddr,
    out Address w0, out Address b0,
    out Address w1, out Address b1,
    out Address w2, out Address b2,
    out Address w3, out Address b3)
{
    int offset = 0;
    w0 = baseAddr.getOffset(offset);
    b0 = baseAddr.getOffset(offset + INPUT_DIM * HIDDEN_DIM);
    offset += L0_PARAMS;
    w1 = baseAddr.getOffset(offset);
    b1 = baseAddr.getOffset(offset + HIDDEN_DIM * HIDDEN_DIM);
    offset += L1_PARAMS;
    w2 = baseAddr.getOffset(offset);
    b2 = baseAddr.getOffset(offset + HIDDEN_DIM * HIDDEN_DIM);
    offset += L2_PARAMS;
    w3 = baseAddr.getOffset(offset);
    b3 = baseAddr.getOffset(offset + HIDDEN_DIM * OUTPUT_DIM);
}

bool isFinite(float x) { return !isnan(x) && !isinf(x); }

// Test 1: Verify forward pass produces finite, non-trivial output
bool testForwardPass()
{
    initializeWeights();

    let baseAddr = Address(params);

    float[3] inputArr = { 0.5f, 0.3f, 0.15f };
    let input = InputVec(inputArr);

    let output = mlpForwardNoDiff(baseAddr, input);

    bool allFinite = isFinite(output[0]) && isFinite(output[1]) && isFinite(output[2]);
    bool notAllZero = (abs(output[0]) > 1e-10f) || (abs(output[1]) > 1e-10f) || (abs(output[2]) > 1e-10f);

    return allFinite && notAllZero;
}

// Test 2: Verify parameter count is correct
bool testParameterCount()
{
    bool l0ok = Layer0.ParameterCount == L0_PARAMS;
    bool l1ok = Layer1.ParameterCount == L1_PARAMS;
    bool l2ok = Layer2.ParameterCount == L2_PARAMS;
    bool l3ok = Layer3.ParameterCount == L3_PARAMS;

    bool nextOk = Layer0.nextOffset(0) == L0_PARAMS;
    nextOk = nextOk && Layer1.nextOffset(L0_PARAMS) == (L0_PARAMS + L1_PARAMS);

    return l0ok && l1ok && l2ok && l3ok && nextOk;
}

// Test 3: Verify backward pass produces finite gradients
bool testBackwardPass()
{
    initializeWeights();
    clearGradients();

    let baseAddr = Address(params);
    let gradBaseAddr = Address(gradParams);

    // Pre-compute all layer addresses outside the differentiable function
    Address pw0, pb0, pw1, pb1, pw2, pb2, pw3, pb3;
    computeLayerAddresses(baseAddr, pw0, pb0, pw1, pb1, pw2, pb2, pw3, pb3);

    Address gw0, gb0, gw1, gb1, gw2, gb2, gw3, gb3;
    computeLayerAddresses(gradBaseAddr, gw0, gb0, gw1, gb1, gw2, gb2, gw3, gb3);

    // Create DifferentialPtrPair for each address
    var w0Pair = DifferentialPtrPair<Address>(pw0, gw0);
    var b0Pair = DifferentialPtrPair<Address>(pb0, gb0);
    var w1Pair = DifferentialPtrPair<Address>(pw1, gw1);
    var b1Pair = DifferentialPtrPair<Address>(pb1, gb1);
    var w2Pair = DifferentialPtrPair<Address>(pw2, gw2);
    var b2Pair = DifferentialPtrPair<Address>(pb2, gb2);
    var w3Pair = DifferentialPtrPair<Address>(pw3, gw3);
    var b3Pair = DifferentialPtrPair<Address>(pb3, gb3);

    float[3] inputArr = { 0.5f, 0.3f, 0.15f };
    let input = InputVec(inputArr);
    var inputPair = diffPair(input);
    let dOutput = OutputVec(1.0f);

    bwd_diff(mlpForwardDiff)(w0Pair, b0Pair, w1Pair, b1Pair, w2Pair, b2Pair, w3Pair, b3Pair, inputPair, dOutput);

    // Check that at least some gradients are non-zero and all are finite
    bool hasNonZeroGrad = false;
    bool allFinite = true;
    for (int i = 0; i < TOTAL_PARAMS; i++)
    {
        float g = gradParams[i];
        if (!isFinite(g)) allFinite = false;
        if (abs(g) > 1e-12f) hasNonZeroGrad = true;
    }

    // Check input gradients are finite
    bool inputGradsFinite = isFinite(inputPair.d[0]) && isFinite(inputPair.d[1]) && isFinite(inputPair.d[2]);

    return allFinite && hasNonZeroGrad && inputGradsFinite;
}

// Test 4: Verify different inputs produce different outputs
bool testDifferentInputsDifferentOutputs()
{
    initializeWeights();

    let baseAddr = Address(params);

    float[3] arr1 = { 0.1f, 0.2f, 0.02f };
    float[3] arr2 = { 0.8f, 0.7f, 0.56f };
    let input1 = InputVec(arr1);
    let input2 = InputVec(arr2);

    let out1 = mlpForwardNoDiff(baseAddr, input1);
    let out2 = mlpForwardNoDiff(baseAddr, input2);

    bool diffCh0 = abs(out1[0] - out2[0]) > 1e-6f;
    bool diffCh1 = abs(out1[1] - out2[1]) > 1e-6f;
    bool diffCh2 = abs(out1[2] - out2[2]) > 1e-6f;

    return diffCh0 || diffCh1 || diffCh2;
}

// Test 5: Verify HasBias=false layer variant compiles and runs
bool testNoBiasVariant()
{
    typealias NoBiasLayer = FFLayer<float, InputVec, HiddenVec, LinearLayout, HiddenAct, false>;

    // NoBias parameter count should exclude bias terms
    bool countOk = NoBiasLayer.ParameterCount == (HIDDEN_DIM * INPUT_DIM);

    initializeWeights();
    let baseAddr = Address(params);

    let w0 = baseAddr.getOffset(0);
    float[3] inputArr = { 0.5f, 0.3f, 0.15f };
    let input = InputVec(inputArr);

    let act = makeHiddenActivation();
    let output = NoBiasLayer(act).eval<Address>(input, w0);

    bool outputFinite = true;
    for (int i = 0; i < HIDDEN_DIM; i++)
    {
        if (!isFinite(output[i])) outputFinite = false;
    }

    return countOk && outputFinite;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    testResult[0] = testForwardPass();
    testResult[1] = testParameterCount();
    testResult[2] = testBackwardPass();
    testResult[3] = testDifferentInputsDifferentOutputs();
    testResult[4] = testNoBiasVariant();

    // BUFFER: 1
    // BUFFER: 1
    // BUFFER: 1
    // BUFFER: 1
    // BUFFER: 1
}
