// Vulkan tests — float only (Vulkan lacks half atomicAdd needed by backward pass)
// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -xslang -DTEST_HALF=0 -xslang -DTEST_POINTER=0 -emit-spirv-directly
// TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -xslang -DTEST_HALF=0 -xslang -DTEST_POINTER=1 -emit-spirv-directly

// CUDA tests — bindless
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature -xslang -DTEST_HALF=0 -xslang -DTEST_POINTER=0
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature -xslang -DTEST_HALF=1 -xslang -DTEST_POINTER=0

// CUDA tests — pointer
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature -xslang -DTEST_HALF=0 -xslang -DTEST_POINTER=1
// TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature -xslang -DTEST_HALF=1 -xslang -DTEST_POINTER=1

#pragma warning(disable: 41017)
import slang.neural;
import common;

#if TEST_HALF
typealias ElementType = half;
#else
typealias ElementType = float;
#endif

//TEST_INPUT: ubuffer(data=[0 0 0 0], stride=4):out,name=testResult
RWStructuredBuffer<uint> testResult;

// Weight stored in tiled layout (16x16 tile, only top-left 2x4 has data):
//   Row 0: 1 2 3 4 0 0 0 0 0 0 0 0 0 0 0 0
//   Row 1: 5 6 7 8 0 0 0 0 0 0 0 0 0 0 0 0
//   Row 2-15: all zeros
// In tiled layout (single 16x16 tile), element (row, col) is at offset row*16 + col.
// So: parameters[0..3] = {1,2,3,4}, parameters[16..19] = {5,6,7,8}, rest = 0.
//
// Bias = {9, 10} stored separately at offset 256.

// Float source for parameter initialization
//TEST_INPUT: set parametersFloat = ubuffer(data=[1.0 2.0 3.0 4.0 5.0 6.0 7.0 8.0 9.0 10.0], stride=4)
uniform RWStructuredBuffer<float> parametersFloat;

// Tiled layout weight buffer: 258 elements (one 16x16 tile + 2 bias).
// Use stride=4 so the buffer is large enough for both half (258*2=516B) and float (258*4=1032B).
//TEST_INPUT: set parameters = ubuffer(stride=4, count=258)

// Tiled layout dW buffer (same layout as parameters).
//TEST_INPUT: set dParameters = ubuffer(stride=4, count=258)

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
static const int workgroupCount = BatchSize / SubgroupSize;

typealias ShMemSize = SharedMemorySize<ElementType, TargetEnum.CUDA, ExecutionMode.Training, SubgroupSize, workgroupCount>;
typealias ShMemSizeLayer1 = ShMemSize.OfLayer1<InputSize, OutputSize>;

void setupParameters(uint tid)
{
    if (tid == 0)
    {
        for (int i = 0; i < 258; i++)
            parameters[i] = ElementType(0.0);

        for (int col = 0; col < InputSize; col++)
            parameters[tiledWeightOffset(0, col)] = ElementType(parametersFloat[col]);

        for (int col = 0; col < InputSize; col++)
            parameters[tiledWeightOffset(1, col)] = ElementType(parametersFloat[InputSize + col]);

        parameters[256] = ElementType(parametersFloat[8]);
        parameters[257] = ElementType(parametersFloat[9]);
    }
}

void cleanupDParameters(uint tid)
{
    if (tid == 0)
    {
        for (int i = 0; i < 258; i++)
            dParameters[i] = ElementType(0.0);
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

[Differentiable]
OutputVector MatVecMul<InputVector, OutputVector>(
    InputVector input,
    Address address)
    where InputVector : IVector<ElementType>
    where OutputVector : IVector<ElementType>
{
    return input.linearTransform<Address, OptimalLayout, OutputVector>(address);
}

[Differentiable]
OutputVector MatVecMulAdd<InputVector, OutputVector>(
    InputVector input,
    Address weightAddress,
    Address biasAddress)
    where InputVector : IVector<ElementType>
    where OutputVector : IVector<ElementType>
{
    return input.linearTransform<Address, OptimalLayout, OutputVector>(weightAddress, biasAddress);
}

// Backward test without bias: forward + backward pass
void BackwardTestWithoutBias(int tid, int resIndex)
{
    typealias InVectorType = WaveTangledVector<ElementType, ShMemSizeLayer1, InputSize, SubgroupSize, workgroupCount>;
    typealias OutVectorType = WaveTangledVector<ElementType, ShMemSizeLayer1, OutputSize, SubgroupSize, workgroupCount>;

    ElementType[InputSize] inputData = { ElementType(1.0), ElementType(2.0), ElementType(3.0), ElementType(4.0) };
    InVectorType input = InVectorType(inputData);

    Address weightAddress = Address(parameters);
    Address dWeightAddress = Address(dParameters);

    let outputVec = MatVecMul<InVectorType, OutVectorType>(input, weightAddress);

    // Forward: [30, 70]
    bool isPassed = true;
    isPassed = isPassed && equals(outputVec[0], ElementType(30.0)) && equals(outputVec[1], ElementType(70.0));

    var weightDiffPair = DifferentialPtrPair<Address>(weightAddress, dWeightAddress);
    let dRes = OutVectorType(1.0f);
    var dPair = diffPair(input);

    // Backward: dInput = W^T * dOutput = [[1,5],[2,6],[3,7],[4,8]] * [1,1] = [6, 8, 10, 12]
    bwd_diff(MatVecMul<InVectorType, OutVectorType>)(dPair, weightDiffPair, dRes);

    isPassed = isPassed &&
        equals(dPair.d[0], ElementType(6.0)) && equals(dPair.d[1], ElementType(8.0)) &&
        equals(dPair.d[2], ElementType(10.0)) && equals(dPair.d[3], ElementType(12.0));

    // dW = dOutput * input^T = [[1],[1]] * [1,2,3,4] = [[1,2,3,4],[1,2,3,4]]
    // Accumulated across 32 threads: 32x.
    isPassed = isPassed &&
        equals(dParameters[tiledDWeightOffset(0, 0)], ElementType(32.0)) && equals(dParameters[tiledDWeightOffset(0, 1)], ElementType(64.0)) &&
        equals(dParameters[tiledDWeightOffset(0, 2)], ElementType(96.0)) && equals(dParameters[tiledDWeightOffset(0, 3)], ElementType(128.0)) &&
        equals(dParameters[tiledDWeightOffset(1, 0)], ElementType(32.0)) && equals(dParameters[tiledDWeightOffset(1, 1)], ElementType(64.0)) &&
        equals(dParameters[tiledDWeightOffset(1, 2)], ElementType(96.0)) && equals(dParameters[tiledDWeightOffset(1, 3)], ElementType(128.0));

    isPassed = WaveActiveAllTrue(isPassed);
    if (tid == 0)
    {
        if (!isPassed)
            printf("BackwardTestWithoutBias FAIL\n");
        testResult[resIndex] = isPassed ? 1 : 0;
    }
}

// Backward test with bias: forward + backward pass
void BackwardTestWithBias(int tid, int resIndex)
{
    typealias InVectorType = WaveTangledVector<ElementType, ShMemSizeLayer1, InputSize, SubgroupSize, 1>;
    typealias OutVectorType = WaveTangledVector<ElementType, ShMemSizeLayer1, OutputSize, SubgroupSize, 1>;

    ElementType[InputSize] inputData = { ElementType(1.0), ElementType(2.0), ElementType(3.0), ElementType(4.0) };
    InVectorType input = InVectorType(inputData);

    Address weightAddress = Address(parameters);
    Address biasAddress = weightAddress.getOffset(256);
    Address dWeightAddress = Address(dParameters);
    Address dBiasAddress = dWeightAddress.getOffset(256);

    let outputVec = MatVecMulAdd<InVectorType, OutVectorType>(input, weightAddress, biasAddress);

    // Forward: [39, 80]
    bool isPassed = true;
    isPassed = isPassed && equals(outputVec[0], ElementType(39.0)) && equals(outputVec[1], ElementType(80.0));

    var weightDiffPair = DifferentialPtrPair<Address>(weightAddress, dWeightAddress);
    var biasDiffPair = DifferentialPtrPair<Address>(biasAddress, dBiasAddress);
    let dOutput = OutVectorType(1.0);
    var dPair = diffPair(input);

    // Backward: dInput = W^T * dOutput = [6, 8, 10, 12]
    bwd_diff(MatVecMulAdd<InVectorType, OutVectorType>)(dPair, weightDiffPair, biasDiffPair, dOutput);

    isPassed = isPassed &&
        equals(dPair.d[0], ElementType(6.0)) && equals(dPair.d[1], ElementType(8.0)) &&
        equals(dPair.d[2], ElementType(10.0)) && equals(dPair.d[3], ElementType(12.0));

    // dW: same as BackwardTestWithoutBias.
    isPassed = isPassed &&
        equals(dParameters[tiledDWeightOffset(0, 0)], ElementType(32.0)) && equals(dParameters[tiledDWeightOffset(0, 1)], ElementType(64.0)) &&
        equals(dParameters[tiledDWeightOffset(0, 2)], ElementType(96.0)) && equals(dParameters[tiledDWeightOffset(0, 3)], ElementType(128.0)) &&
        equals(dParameters[tiledDWeightOffset(1, 0)], ElementType(32.0)) && equals(dParameters[tiledDWeightOffset(1, 1)], ElementType(64.0)) &&
        equals(dParameters[tiledDWeightOffset(1, 2)], ElementType(96.0)) && equals(dParameters[tiledDWeightOffset(1, 3)], ElementType(128.0));

    // dBias = dOutput accumulated across 32 threads = [32, 32]
    isPassed = isPassed &&
        equals(dParameters[256], ElementType(32.0)) && equals(dParameters[257], ElementType(32.0));

    isPassed = WaveActiveAllTrue(isPassed);
    if (tid == 0)
    {
        if (!isPassed)
            printf("BackwardTestWithBias FAIL\n");
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
    // BUFFER-NEXT: 1

    cleanupDParameters(tid);
    AllMemoryBarrierWithGroupSync();
    BackwardTestWithoutBias(tid, 2);
    // BUFFER-NEXT: 1

    cleanupDParameters(tid);
    AllMemoryBarrierWithGroupSync();
    BackwardTestWithBias(tid, 3);
    // BUFFER-NEXT: 1
}
