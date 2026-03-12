// // TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -xslang -DTEST_HALF=0 -xslang -DTEST_POINTER=0 -emit-spirv-directly
// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature -xslang -DTEST_HALF=0 -xslang -DTEST_POINTER=0
// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature -xslang -DTEST_HALF=1 -xslang -DTEST_POINTER=0
// // TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature -xslang -DTEST_HALF=0 -xslang -DTEST_POINTER=1
// // TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature -xslang -DTEST_HALF=1 -xslang -DTEST_POINTER=1

import slang.neural;
import common;

#if TEST_HALF
typealias ElementType = half;
#else
typealias ElementType = float;
#endif

//TEST_INPUT: ubuffer(data=[0 0], stride=4):out,name=testResult
RWStructuredBuffer<uint> testResult;

// Weight stored in tiled layout (16x16 tile, only top-left 2x4 has data):
//   Row 0: 1 2 3 4 0 0 0 0 0 0 0 0 0 0 0 0
//   Row 1: 5 6 7 8 0 0 0 0 0 0 0 0 0 0 0 0
//   Row 2-15: all zeros
// In tiled layout (single 16x16 tile), element (row, col) is at offset row*16 + col.
// So: parameters[0..3] = {1,2,3,4}, parameters[16..19] = {5,6,7,8}, rest = 0.
//
// Bias = {9, 10} stored separately.

// Float source for parameter initialization
//TEST_INPUT: set parametersFloat = ubuffer(data=[1.0 2.0 3.0 4.0 5.0 6.0 7.0 8.0 9.0 10.0], stride=4)
uniform RWStructuredBuffer<float> parametersFloat;

// Tiled layout weight buffer (256 elements for one 16x16 tile + 2 for bias)
//TEST_INPUT: set parameters = ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=2)

#if TEST_POINTER
    uniform ElementType* parameters;
    typealias Address = PointerAddress<ElementType>;
#else
    uniform RWStructuredBuffer<ElementType>.Handle parameters;
    typealias Address = BindlessAddress<ElementType>;
#endif

static const int InputSize = 4;
static const int OutputSize = 2;
static const int BatchSize = 32;
static const int SubgroupSize = 32;
static const int workgroupCount = BatchSize / SubgroupSize;

// Shared memory size uses padded dimensions because TiledMMAHelper requires tile-aligned sizes.
typealias ShMemSize = SharedMemorySize<ElementType, TargetEnum.CUDA, ExecutionMode.Inference, SubgroupSize, BatchSize / SubgroupSize>;
typealias ShMemSizeLayer1 = ShMemSize.OfLayer1<InputSize, OutputSize>;

// Set up weight in tiled layout and bias.
// Tiled layout for a 16x16 tile: element (row, col) at offset row * 16 + col.
// Weight matrix (2x4):
//   row 0: 1 2 3 4  → parameters[0..3]
//   row 1: 5 6 7 8  → parameters[16..19]
// Bias: {9, 10} → parameters[256], parameters[257]
void setupParameters(uint tid)
{
    if (tid == 0)
    {
        // Clear tile
        for (int i = 0; i < 258; i++)
            parameters[i] = ElementType(0.0);

        // Row 0 of weight: parameters[0..3] = {1, 2, 3, 4}
        for (int col = 0; col < InputSize; col++)
            parameters[col] = ElementType(parametersFloat[col]);

        // Row 1 of weight: parameters[16..19] = {5, 6, 7, 8}
        for (int col = 0; col < InputSize; col++)
            parameters[16 + col] = ElementType(parametersFloat[InputSize + col]);

        // Bias at offset 256 (after the 16x16 tile): {9, 10}
        parameters[256] = ElementType(parametersFloat[8]);
        parameters[257] = ElementType(parametersFloat[9]);
    }
}

// Forward-only test without bias
void ForwardTestWithoutBias(int tid, int resIndex)
{
    typealias InVectorType = WaveTangledVector<ElementType, ShMemSizeLayer1, InputSize, SubgroupSize, 1>;
    typealias OutVectorType = WaveTangledVector<ElementType, ShMemSizeLayer1, OutputSize, SubgroupSize, 1>;

    ElementType[InputSize] inputData = { ElementType(1.0), ElementType(2.0), ElementType(3.0), ElementType(4.0) };
    InVectorType input = InVectorType(inputData);

    Address weightAddress = Address(parameters);

    let outputVec = input.linearTransform<Address, OptimalLayout, OutVectorType>(weightAddress);

    // Weight * input = [[1,2,3,4],[5,6,7,8]] * [1,2,3,4]
    // = [1*1+2*2+3*3+4*4, 5*1+6*2+7*3+8*4] = [30, 70]
    bool isPassed = true;
    isPassed = isPassed && equals(outputVec[0], ElementType(30.0)) && equals(outputVec[1], ElementType(70.0));

    isPassed = WaveActiveAllTrue(isPassed);
    if (tid == 0)
    {
        if (!isPassed)
            printf("ForwardTestWithoutBias FAIL: got [%f, %f], expected [30, 70]\n",
                float(outputVec[0]), float(outputVec[1]));
        testResult[resIndex] = isPassed ? 1 : 0;
    }
}

// Forward-only test with bias
void ForwardTestWithBias(int tid, int resIndex)
{
    typealias InVectorType = WaveTangledVector<ElementType, ShMemSizeLayer1, InputSize, SubgroupSize, 1>;
    typealias OutVectorType = WaveTangledVector<ElementType, ShMemSizeLayer1, OutputSize, SubgroupSize, 1>;

    ElementType[InputSize] inputData = { ElementType(1.0), ElementType(2.0), ElementType(3.0), ElementType(4.0) };
    InVectorType input = InVectorType(inputData);

    Address weightAddress = Address(parameters);
    Address biasAddress = weightAddress.getOffset(256);

    let outputVec = input.linearTransform<Address, OptimalLayout, OutVectorType>(weightAddress, biasAddress);

    // Weight * input + bias = [30 + 9, 70 + 10] = [39, 80]
    bool isPassed = true;
    isPassed = isPassed && equals(outputVec[0], ElementType(39.0)) && equals(outputVec[1], ElementType(80.0));

    isPassed = WaveActiveAllTrue(isPassed);
    if (tid == 0)
    {
        if (!isPassed)
            printf("ForwardTestWithBias FAIL: got [%f, %f], expected [39, 80]\n",
                float(outputVec[0]), float(outputVec[1]));
        testResult[resIndex] = isPassed ? 1 : 0;
    }
}

[shader("compute")]
[numthreads(BatchSize, 1, 1)]
void computeMain(uint tid : SV_DispatchThreadID)
{
    setupParameters(tid);
    AllMemoryBarrierWithGroupSync();

    ForwardTestWithoutBias(tid, 0);
    // BUFFER: 1

    ForwardTestWithBias(tid, 1);
    // BUFFER: 1
}
