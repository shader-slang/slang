// Test FFLayer with WaveTangledVector using OptimalLayout (tiled 16x16 storage).
// This verifies that FFLayer works with tiled weight layout for CooperativeMatrix acceleration.
//
// OptimalLayout (tiled) — single-warp:
// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -xslang -DTEST_POINTER=0 -xslang -DWARP_COUNT=1 -emit-spirv-directly
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -xslang -experimental-feature -xslang -DTEST_POINTER=0 -xslang -DWARP_COUNT=1
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -xslang -experimental-feature -xslang -DTEST_POINTER=1 -xslang -DWARP_COUNT=1
// TEST(compute, metal):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-mtl -compute -shaderobj -output-using-type -xslang -experimental-feature -xslang -DTEST_POINTER=0 -xslang -DWARP_COUNT=1 -xslang -DTEST_METAL=1

#pragma warning(disable: 41017)
import slang.neural;
import common;

#ifndef TEST_METAL
#define TEST_METAL 0
#endif

#ifndef WARP_COUNT
#define WARP_COUNT 1
#endif

#ifndef TEST_POINTER
#define TEST_POINTER 0
#endif

typealias ElementType = float;

//TEST_INPUT: set parametersPtr = ubuffer(stride=4, count=258)
//TEST_INPUT: set dParametersPtr = ubuffer(stride=4, count=258)
//TEST_INPUT: set buffers.parameters = ubuffer(stride=4, count=258)
//TEST_INPUT: set buffers.dParameters = ubuffer(stride=4, count=258)

static const int BiasOffset = 256;
static const int ParamBufferSize = 258;
typealias Layout = OptimalLayout;

//TEST_INPUT: ubuffer(data=[0 0 0], stride=4):out,name=testResult
RWStructuredBuffer<uint> testResult;

uniform ElementType* parametersPtr;
uniform ElementType* dParametersPtr;

struct TestBuffers
{
    RWStructuredBuffer<ElementType>.Handle parameters;
    RWStructuredBuffer<ElementType>.Handle dParameters;
}

ParameterBlock<TestBuffers> buffers;

#if TEST_POINTER
    #define parameters parametersPtr
    #define dParameters dParametersPtr
    typealias Address = PointerAddress<ElementType>;
#else
    #define parameters buffers.parameters
    #define dParameters buffers.dParameters
    typealias Address = BindlessAddress<ElementType>;
#endif

static const int InputSize = 4;
static const int OutputSize = 2;
static const int SubgroupSize = 32;
static const int WarpCount = WARP_COUNT;
static const int BatchSize = WarpCount * SubgroupSize;

#if TEST_METAL
#define TEST_TARGET TargetEnum.Metal
#else
#define TEST_TARGET TargetEnum.CUDA
#endif

typealias ShMemSize = SharedMemorySize<ElementType, TEST_TARGET, ExecutionMode.Training, SubgroupSize, WarpCount>;
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
    AllMemoryBarrierWithGroupSync();

    // dInput = W^T * dOutput = [6, 8, 10, 12]
    bool isPassed = true;
    isPassed = isPassed && equals(inputPair.d[0], ElementType(6.0));
    isPassed = isPassed && equals(inputPair.d[1], ElementType(8.0));
    isPassed = isPassed && equals(inputPair.d[2], ElementType(10.0));
    isPassed = isPassed && equals(inputPair.d[3], ElementType(12.0));

    // dW accumulated across BatchSize threads.
    // dW = BatchSize * [[1,2,3,4],[1,2,3,4]]
    ElementType scale = ElementType(BatchSize);
    isPassed = isPassed && equals(dParameters[tiledDWeightOffset(0, 0)], scale * ElementType(1.0));
    isPassed = isPassed && equals(dParameters[tiledDWeightOffset(0, 1)], scale * ElementType(2.0));
    isPassed = isPassed && equals(dParameters[tiledDWeightOffset(0, 2)], scale * ElementType(3.0));
    isPassed = isPassed && equals(dParameters[tiledDWeightOffset(0, 3)], scale * ElementType(4.0));
    isPassed = isPassed && equals(dParameters[tiledDWeightOffset(1, 0)], scale * ElementType(1.0));
    isPassed = isPassed && equals(dParameters[tiledDWeightOffset(1, 1)], scale * ElementType(2.0));
    isPassed = isPassed && equals(dParameters[tiledDWeightOffset(1, 2)], scale * ElementType(3.0));
    isPassed = isPassed && equals(dParameters[tiledDWeightOffset(1, 3)], scale * ElementType(4.0));

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

        for (int col = 0; col < InputSize; col++)
            parameters[tiledWeightOffset(0, col)] = ElementType(col + 1);

        for (int col = 0; col < InputSize; col++)
            parameters[tiledWeightOffset(1, col)] = ElementType(InputSize + col + 1);

        parameters[256] = ElementType(9);
        parameters[257] = ElementType(10);
    }
}

[shader("compute")]
[numthreads(BatchSize, 1, 1)]
void computeMain(uint tid : SV_GroupIndex, uint3 gid : SV_GroupID)
{
    if (gid.x != 0)
        return;

    uint lane = WaveGetLaneIndex();
    uint wave = WaveGetWaveIndex();
    if (lane >= uint(SubgroupSize) || wave >= uint(WarpCount))
        return;

    setupParameters(tid);
    AllMemoryBarrierWithGroupSync();

    testFFLayerParameterCount(tid, 0);
    // BUFFER: {{^}}1{{$}}

    testFFLayerForward(tid, 1);
    // BUFFER-NEXT: {{^}}1{{$}}

    cleanupDParameters(tid);
    AllMemoryBarrierWithGroupSync();
    testFFLayerBackward(tid, 2);
    // BUFFER-NEXT: {{^}}1{{$}}
}
