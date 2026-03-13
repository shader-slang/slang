// Test FFLayer with WaveTangledVector using OptimalLayout (tiled 16x16 storage).
// This verifies that FFLayer works with tiled weight layout for CooperativeMatrix acceleration.
//
// OptimalLayout (tiled) — single-warp:
// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -xslang -DTEST_POINTER=0 -xslang -DWARP_COUNT=1 -xslang -DTEST_TENSORVIEW=0 -emit-spirv-directly
// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0 -xslang -experimental-feature -xslang -DTEST_POINTER=0 -xslang -DWARP_COUNT=1 -xslang -DTEST_TENSORVIEW=0
// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0 -xslang -experimental-feature -xslang -DTEST_POINTER=1 -xslang -DWARP_COUNT=1 -xslang -DTEST_TENSORVIEW=0

#pragma warning(disable: 41017)
import slang.neural;
import common;

#ifndef WARP_COUNT
#define WARP_COUNT 1
#endif

#ifndef TEST_POINTER
#define TEST_POINTER 0
#endif

#ifndef TEST_TENSORVIEW
#define TEST_TENSORVIEW 0
#endif

typealias ElementType = float;

//TEST_INPUT: set parametersFloat = ubuffer(data=[1.0 2.0 3.0 4.0 5.0 6.0 7.0 8.0 9.0 10.0], stride=4)
uniform RWStructuredBuffer<float>.Handle parametersFloat;

//TEST_INPUT: set parameters = ubuffer(stride=4, count=258)
//TEST_INPUT: set dParameters = ubuffer(stride=4, count=258)

static const int BiasOffset = 256;
static const int ParamBufferSize = 258;
typealias Layout = OptimalLayout;

//TEST_INPUT: ubuffer(data=[0 0 0], stride=4):out,name=testResult
RWStructuredBuffer<uint> testResult;

#if TEST_TENSORVIEW
    uniform TensorView<ElementType> parameters;
    uniform TensorView<ElementType> dParameters;
    typealias Address = TorchTensorViewAddress<ElementType>;
#elif TEST_POINTER
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
static const int SubgroupSize = 32;
static const int WarpCount = WARP_COUNT;
static const int BatchSize = WarpCount * SubgroupSize;

typealias ShMemSize = SharedMemorySize<ElementType, TargetEnum.CUDA, ExecutionMode.Training, SubgroupSize, WarpCount>;
typealias ShMemSizeLayer1 = ShMemSize.OfLayer1<InputSize, OutputSize>;
typealias InVec = WaveTangledVector<ElementType, ShMemSizeLayer1, InputSize, SubgroupSize, WarpCount>;
typealias OutVec = WaveTangledVector<ElementType, ShMemSizeLayer1, OutputSize, SubgroupSize, WarpCount>;

typealias Layer = FFLayer<ElementType, InVec, OutVec, Layout, IdentityActivation<ElementType>, true>;

[Differentiable]
OutVec computeLayerOutput(Address paramAddr, InVec input, Layer layer)
{
    return layer.eval<Address>(input, paramAddr);
}

void testFFLayerForward(int tid, int resIndex)
{
    // W = [[1,2,3,4],[5,6,7,8]], b = [9, 10]
    // W*x + b = [30+9, 70+10] = [39, 80]
    ElementType[InputSize] inputData = { ElementType(1.0), ElementType(2.0), ElementType(3.0), ElementType(4.0) };
    InVec input = InVec(inputData);

    let baseAddr = Address(parameters);
    let layer = Layer();

    let output = layer.eval<Address>(input, baseAddr);

    bool isPassed = true;
    isPassed = isPassed && equals(output[0], ElementType(39.0));
    isPassed = isPassed && equals(output[1], ElementType(80.0));

    isPassed = WaveActiveAllTrue(isPassed);
    if (tid == 0)
    {
        testResult[resIndex] = isPassed ? 1 : 0;
    }
}

void testFFLayerBackward(int tid, int resIndex)
{
    ElementType[InputSize] inputData = { ElementType(1.0), ElementType(2.0), ElementType(3.0), ElementType(4.0) };
    InVec input = InVec(inputData);

    let baseAddr = Address(parameters);
    let dBaseAddr = Address(dParameters);
    let layer = Layer();

    var paramAddrPair = DifferentialPtrPair<Address>(baseAddr, dBaseAddr);
    var inputPair = diffPair(input);
    let dOutput = OutVec(1.0);

    bwd_diff(computeLayerOutput)(paramAddrPair, inputPair, layer, dOutput);

    // dInput = W^T * dOutput = [6, 8, 10, 12]
    bool isPassed = true;
    isPassed = isPassed && equals(inputPair.d[0], ElementType(6.0));
    isPassed = isPassed && equals(inputPair.d[1], ElementType(8.0));
    isPassed = isPassed && equals(inputPair.d[2], ElementType(10.0));
    isPassed = isPassed && equals(inputPair.d[3], ElementType(12.0));

    // dW accumulated across BatchSize threads.
    // dW = BatchSize * [[1,2,3,4],[1,2,3,4]]
    ElementType scale = ElementType(BatchSize);
    // Tiled layout: row 0 at [0..3], row 1 at [16..19]
    isPassed = isPassed && equals(dParameters[0], scale * ElementType(1.0));
    isPassed = isPassed && equals(dParameters[1], scale * ElementType(2.0));
    isPassed = isPassed && equals(dParameters[2], scale * ElementType(3.0));
    isPassed = isPassed && equals(dParameters[3], scale * ElementType(4.0));
    isPassed = isPassed && equals(dParameters[16], scale * ElementType(1.0));
    isPassed = isPassed && equals(dParameters[17], scale * ElementType(2.0));
    isPassed = isPassed && equals(dParameters[18], scale * ElementType(3.0));
    isPassed = isPassed && equals(dParameters[19], scale * ElementType(4.0));

    // dBias = [BatchSize, BatchSize]
    isPassed = isPassed && equals(dParameters[BiasOffset], scale);
    isPassed = isPassed && equals(dParameters[BiasOffset + 1], scale);

    isPassed = WaveActiveAllTrue(isPassed);
    if (tid == 0)
    {
        testResult[resIndex] = isPassed ? 1 : 0;
    }
}

void testFFLayerParameterCount(int tid, int resIndex)
{
    // For 4->2 layer with bias: 4*2 + 2 = 10
    bool isPassed = (Layer.ParameterCount == 10);

    // OptimalLayout: ceil(2/16)*ceil(4/16)*256 + 2 = 1*1*256 + 2 = 258
    isPassed = isPassed && (Layer.StorageSize == 258);

    isPassed = WaveActiveAllTrue(isPassed);
    if (tid == 0)
    {
        testResult[resIndex] = isPassed ? 1 : 0;
    }
}

void cleanupDParameters(uint tid)
{
    if (tid == 0)
    {
        for (int i = 0; i < ParamBufferSize; i++)
            dParameters[i] = ElementType(0.0);
    }
}

void setupParameters(uint tid)
{
    if (tid == 0)
    {
        for (int i = 0; i < ParamBufferSize; i++)
            parameters[i] = ElementType(0.0);

        // Row 0: parameters[0..3] = {1,2,3,4}
        for (int col = 0; col < InputSize; col++)
            parameters[col] = ElementType(parametersFloat[col]);

        // Row 1: parameters[16..19] = {5,6,7,8}
        for (int col = 0; col < InputSize; col++)
            parameters[16 + col] = ElementType(parametersFloat[InputSize + col]);

        // Bias at offset 256: {9, 10}
        parameters[256] = ElementType(parametersFloat[8]);
        parameters[257] = ElementType(parametersFloat[9]);
    }
}

[shader("compute")]
[numthreads(BatchSize, 1, 1)]
void computeMain(uint tid : SV_DispatchThreadID)
{
    setupParameters(tid);
    AllMemoryBarrierWithGroupSync();

    testFFLayerParameterCount(tid, 0);
    // BUFFER: 1

    testFFLayerForward(tid, 1);
    // BUFFER: 1

    cleanupDParameters(tid);
    AllMemoryBarrierWithGroupSync();
    testFFLayerBackward(tid, 2);
    // BUFFER: 1
}
