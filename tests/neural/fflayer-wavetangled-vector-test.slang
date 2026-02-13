// Test FFLayer with WaveTangledVector (CooperativeMatrix accelerated).
// This verifies that FFLayer works with non-InlineVector types using Layout parameter.
//
// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -xslang -DTEST_POINTER=0 -emit-spirv-directly
// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0 -xslang -experimental-feature -xslang -DTEST_POINTER=0
// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0 -xslang -experimental-feature -xslang -DTEST_POINTER=1

import neural;

typealias ElementType = float;

// set up a 2x4 matrix for input parameters, the last 2 elements are for bias
// W = [[1,2,3,4],[5,6,7,8]], b = [9, 10]
//TEST_INPUT: set parametersFloat = ubuffer(data=[1.0 2.0 3.0 4.0 5.0 6.0 7.0 8.0 9.0 10.0], stride=4)
uniform RWStructuredBuffer<float>.Handle parametersFloat;

//TEST_INPUT: set parameters = ubuffer(data=[0 0 0 0 0 0 0 0 0 0], stride=4)

// Create a buffer to store the test result
//TEST_INPUT: ubuffer(data=[0 0 0], stride=4):out,name=testResult
RWStructuredBuffer<uint> testResult;

//TEST_INPUT: set dParameters = ubuffer(data=[0 0 0 0 0 0 0 0 0 0], stride=4)

#if TEST_POINTER
    uniform ElementType* parameters;
    uniform ElementType* dParameters;
    typealias Address = PointerAddress<ElementType>;
#else
    uniform RWStructuredBuffer<ElementType>.Handle parameters;
    uniform RWStructuredBuffer<ElementType>.Handle dParameters;
    typealias Address = BindlessAddress<ElementType>;
#endif

static const int InputSize = 4;
static const int OutputSize = 2;
static const int BatchSize = 32;
static const int SubgroupSize = 32;

typealias ShMemSize = SharedMemorySize<ElementType, TargetEnum.CUDA, ExecutionMode.Training, SubgroupSize, BatchSize / SubgroupSize>;
typealias ShMemSizeLayer1 = ShMemSize.OfLayer1<InputSize, OutputSize>;
typealias ShMemPool = SharedMemoryPool<ShMemSizeLayer1>;

typealias InVec = WaveTangledVector<ElementType, ShMemPool, InputSize, SubgroupSize>;
typealias OutVec = WaveTangledVector<ElementType, ShMemPool, OutputSize, SubgroupSize>;

// FFLayer with WaveTangledVector and LinearLayout
typealias Layer = FFLayer<ElementType, InVec, OutVec, Address, LinearLayout, IdentityActivation<ElementType>, true>;

// Wrapper function for autodiff
[Differentiable]
OutVec computeLayerOutput(Address weightAddr, Address biasAddr, InVec input, Layer layer)
{
    return layer.eval<Address>(weightAddr, biasAddr, input);
}

// Test FFLayer forward pass with WaveTangledVector
void testFFLayerForward(int tid, int resIndex)
{
    // Input: x = [1, 2, 3, 4]
    // W = [[1,2,3,4],[5,6,7,8]], b = [9, 10]
    // Output: W*x + b = [1*1+2*2+3*3+4*4+9, 5*1+6*2+7*3+8*4+10] = [39, 80]
    
    ElementType[InputSize] inputData = { ElementType(1.0), ElementType(2.0), ElementType(3.0), ElementType(4.0) };
    InVec input = InVec(inputData);
    
    let baseAddr = Address(parameters);
    
    // Create layer with weights at offset 0, bias at offset 8
    let layer = Layer(int(0), int(8));
    
    // Compute weight/bias addresses
    let weightAddr = baseAddr.getOffset(0);
    let biasAddr = baseAddr.getOffset(8);
    
    // Forward pass
    let output = layer.eval<Address>(weightAddr, biasAddr, input);
    
    // Verify output: [39, 80]
    bool isPassed = true;
    isPassed = isPassed && (output[0] == ElementType(39.0));
    isPassed = isPassed && (output[1] == ElementType(80.0));
    
    isPassed = WaveActiveAllTrue(isPassed);
    if (tid == 0)
    {
        testResult[resIndex] = isPassed ? 1 : 0;
    }
}

// Test FFLayer backward pass with WaveTangledVector (autodiff)
void testFFLayerBackward(int tid, int resIndex)
{
    ElementType[InputSize] inputData = { ElementType(1.0), ElementType(2.0), ElementType(3.0), ElementType(4.0) };
    InVec input = InVec(inputData);
    
    let baseAddr = Address(parameters);
    let dBaseAddr = Address(dParameters);
    
    let layer = Layer(int(0), int(8));
    
    // Compute weight/bias addresses outside differentiable function
    let weightAddr = baseAddr.getOffset(0);
    let biasAddr = baseAddr.getOffset(8);
    let dWeightAddr = dBaseAddr.getOffset(0);
    let dBiasAddr = dBaseAddr.getOffset(8);
    
    // Set up differential pairs for weight and bias addresses
    var weightAddrPair = DifferentialPtrPair<Address>(weightAddr, dWeightAddr);
    var biasAddrPair = DifferentialPtrPair<Address>(biasAddr, dBiasAddr);
    var inputPair = diffPair(input);
    
    // dOutput = [1, 1]
    let dOutput = OutVec(1.0);
    
    // Backward pass
    bwd_diff(computeLayerOutput)(weightAddrPair, biasAddrPair, inputPair, layer, dOutput);
    
    // Verify input gradient: dInput = W^T * dOutput = [1+5, 2+6, 3+7, 4+8] = [6, 8, 10, 12]
    bool isPassed = true;
    isPassed = isPassed && (inputPair.d[0] == ElementType(6.0));
    isPassed = isPassed && (inputPair.d[1] == ElementType(8.0));
    isPassed = isPassed && (inputPair.d[2] == ElementType(10.0));
    isPassed = isPassed && (inputPair.d[3] == ElementType(12.0));
    
    // Weight gradients are accumulated across 32 threads
    // dW = dOutput * input^T = [1,1]^T * [1,2,3,4] = [[1,2,3,4],[1,2,3,4]]
    // Accumulated: 32 * [[1,2,3,4],[1,2,3,4]]
    isPassed = isPassed && (dParameters[0] == ElementType(32.0));
    isPassed = isPassed && (dParameters[1] == ElementType(64.0));
    isPassed = isPassed && (dParameters[2] == ElementType(96.0));
    isPassed = isPassed && (dParameters[3] == ElementType(128.0));
    isPassed = isPassed && (dParameters[4] == ElementType(32.0));
    isPassed = isPassed && (dParameters[5] == ElementType(64.0));
    isPassed = isPassed && (dParameters[6] == ElementType(96.0));
    isPassed = isPassed && (dParameters[7] == ElementType(128.0));
    
    // Bias gradients: dBias = dOutput = [1, 1], accumulated: [32, 32]
    isPassed = isPassed && (dParameters[8] == ElementType(32.0));
    isPassed = isPassed && (dParameters[9] == ElementType(32.0));
    
    isPassed = WaveActiveAllTrue(isPassed);
    if (tid == 0)
    {
        testResult[resIndex] = isPassed ? 1 : 0;
    }
}

// Test FFLayer ParameterCount with WaveTangledVector
void testFFLayerParameterCount(int tid, int resIndex)
{
    // For 4->2 layer with bias: 4*2 + 2 = 10
    bool isPassed = (Layer.ParameterCount == 10);
    
    isPassed = WaveActiveAllTrue(isPassed);
    if (tid == 0)
    {
        testResult[resIndex] = isPassed ? 1 : 0;
    }
}

void cleanupDParameters()
{
    for (int i = 0; i < 10; i++)
    {
        dParameters[i] = ElementType(0.0);
    }
}

void setupParameters(uint tid)
{
    if (tid == 0)
    {
        for (int i = 0; i < 10; i++)
        {
            parameters[i] = ElementType(parametersFloat[i]);
        }
    }
}

[shader("compute")]
[numthreads(BatchSize, 1, 1)]
void computeMain(uint tid : SV_DispatchThreadID)
{
    setupParameters(tid);
    GroupMemoryBarrierWithWaveSync();
    
    testFFLayerParameterCount(tid, 0);
    // BUFFER: 1
    
    testFFLayerForward(tid, 1);
    // BUFFER: 1
    
    cleanupDParameters();
    testFFLayerBackward(tid, 2);
    // BUFFER: 1
}
