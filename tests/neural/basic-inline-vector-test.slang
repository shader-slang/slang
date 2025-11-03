//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -xslang -DTEST_HALF=0
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-dx12 -compute -shaderobj -profile cs_6_6 -xslang -experimental-feature -output-using-type -xslang -DTEST_HALF=0
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-mtl -compute -shaderobj -output-using-type -xslang -experimental-feature -xslang -DTEST_HALF=0

// Currently, only CUDA supports atomicAdd on half. So we can only test fp16 on CUDA.
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature -xslang -DTEST_HALF=0
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0 -xslang -experimental-feature -xslang -DTEST_HALF=1
import neural;

#if TEST_HALF
typealias ElementType = half;
#else
typealias ElementType = float;
#endif

// set up a 2x4 matrix for input parameters, the last 2 elements are for bias
//TEST_INPUT: ubuffer(data=[1.0 2.0 3.0 4.0 5.0 6.0 7.0 8.0 9.0 10.0], stride=4):name=parametersFloat
// 1 2 3 4
// 5 6 7 8
// bias = {9.0, 10.0}
RWStructuredBuffer<float> parametersFloat;

//TEST_INPUT: ubuffer(data=[0 0 0 0 0 0 0 0 0 0], stride=2):name=parameters
RWStructuredBuffer<ElementType> parameters;

// Create a buffer to store the test result
//TEST_INPUT: ubuffer(data=[0 0], stride=4):out,name=testResult
RWStructuredBuffer<uint> testResult;

//TEST_INPUT: ubuffer(data=[0 0 0 0], stride=4):name=dInput
RWStructuredBuffer<ElementType> dInput;

// set up a 2x4 matrix for derivative of parameters, the last 2 elements are for derivative of bias
//TEST_INPUT: ubuffer(data=[0 0 0 0 0 0 0 0 0 0], stride=4):name=dParameters
RWStructuredBuffer<ElementType> dParameters;

typealias BufferStorage = StructuredBufferStorage<ElementType>;

[Differentiable]
OutputVector TestInlineVectorMatMul<InputVector, OutputVector>(
    InputVector input,
    BufferStorage weightStorage,
    BufferStorage.Address weightAddress)
    where InputVector : IVector<ElementType, 4>
    where OutputVector : IVector<ElementType, 2>
{
    var outputVec = input.linearTransform<2, BufferStorage, OutputVector>(weightStorage, weightAddress);
    return outputVec;
}

[Differentiable]
OutputVector TestInlineVectorMatMulAdd<InputVector, OutputVector>(
    InputVector input,
    BufferStorage weightStorage,
    BufferStorage biasStorage,
    BufferStorage.Address weightAddress,
    BufferStorage.Address biasAddress)
    where InputVector : IVector<ElementType, 4>
    where OutputVector : IVector<ElementType, 2>
{
    var outputVec = input.linearTransform<2, BufferStorage, OutputVector>(weightStorage, biasStorage, weightAddress, biasAddress);
    return outputVec;
}

// Basic test on MatMul without bias, this test covers both forward and backward pass
bool BasicTestWithoutBias()
{
    ElementType[4] inputData = {ElementType(1.0), ElementType(2.0), ElementType(3.0), ElementType(4.0)};
    let input = InlineVector<ElementType, 4>(inputData);
    BufferStorage weightStorage = BufferStorage(parameters);
    BufferStorage dweightStorage = BufferStorage(dParameters);
    BufferStorage.Address weightAddress = 0;

    // Run the forward pass
    let outputVec = TestInlineVectorMatMul<InlineVector<ElementType, 4>, InlineVector<ElementType, 2>>(input, weightStorage, weightAddress);

    // // (1*1 + 2*2 + 3*3 + 4*4) = 30.0
    // // (5*1 + 6*2 + 7*3 + 8*4) = 70.0
    bool isPassed = (outputVec[0] == 30.0 && outputVec[1] == 70.0);


    var weightDiffPair = DifferentialPtrPair<BufferStorage>(weightStorage, dweightStorage);
    let dRes = InlineVector<ElementType, 2>(1.0f);
    var dPair = diffPair(input);

    // Run the backward pass
    // dInput = W^T * dOutput
    // dInput = {3, 7, 11, 15}
    bwd_diff(TestInlineVectorMatMul<InlineVector<ElementType, 4>, InlineVector<ElementType, 2>>)
                (dPair, weightDiffPair, weightAddress, dRes);

    isPassed = isPassed &&
        dPair.d[0] == 3.0 && dPair.d[1] == 7.0 && dPair.d[2] == 11.0 && dPair.d[3] == 15.0;

    // dW = dOutput * dInput^T
    // dW = [1, 1]^T * [1, 2, 3, 4]
    //    = [[1, 2, 3, 4]; [1, 2, 3, 4]]
    isPassed = isPassed &&
        dParameters[0] == 1.0 && dParameters[1] == 2.0 && dParameters[2] == 3.0 && dParameters[3] == 4.0 &&
        dParameters[4] == 1.0 && dParameters[5] == 2.0 && dParameters[6] == 3.0 && dParameters[7] == 4.0;

    return isPassed;
}

// Basic test on MatMul with bias, this test covers both forward and backward pass
bool BasicTestWithBias()
{
    ElementType[4] inputData = {ElementType(1.0), ElementType(2.0), ElementType(3.0), ElementType(4.0)};
    let input = InlineVector<ElementType, 4>(inputData);
    BufferStorage weightStorage = BufferStorage(parameters);
    BufferStorage biasStorage = BufferStorage(parameters);
    BufferStorage dweightStorage = BufferStorage(dParameters);
    BufferStorage dbiasStorage = BufferStorage(dParameters);

    // weight and bias are stored in the same buffer, we just use the different address for them
    BufferStorage.Address weightAddress = 0;
    BufferStorage.Address biasAddress = 8;

    // Run the forward pass
    let outputVec =
        TestInlineVectorMatMulAdd<InlineVector<ElementType, 4>, InlineVector<ElementType, 2>>
            (input, weightStorage, biasStorage, weightAddress, biasAddress);

    // // (1*1 + 2*2 + 3*3 + 4*4) + 9.0 = 39.0
    // // (5*1 + 6*2 + 7*3 + 8*4) + 10.0 = 80.0
    bool isPassed = (outputVec[0] == 39.0 && outputVec[1] == 80.0);

    var weightDiffPair = DifferentialPtrPair<BufferStorage>(weightStorage, dweightStorage);
    var biasDiffPair = DifferentialPtrPair<BufferStorage>(biasStorage, dbiasStorage);
    let dOutput = InlineVector<ElementType, 2>(1.0);
    var dPair = diffPair(input);

    // Run the backward pass
    // dInput = W^T * dOutput
    // dInput = {3, 7, 11, 15}
    bwd_diff(TestInlineVectorMatMulAdd<InlineVector<ElementType, 4>, InlineVector<ElementType, 2>>)
                (dPair, weightDiffPair, biasDiffPair, weightAddress, biasAddress, dOutput);

    isPassed = isPassed &&
        dPair.d[0] == 3.0 && dPair.d[1] == 7.0 && dPair.d[2] == 11.0 && dPair.d[3] == 15.0;

    // dW = dOutput * dInput^T
    // dW = [1, 1]^T * [1, 2, 3, 4]
    //    = [[1, 2, 3, 4]; [1, 2, 3, 4]]
    isPassed = isPassed &&
        dParameters[0] == 1.0 && dParameters[1] == 2.0 && dParameters[2] == 3.0 && dParameters[3] == 4.0 &&
        dParameters[4] == 1.0 && dParameters[5] == 2.0 && dParameters[6] == 3.0 && dParameters[7] == 4.0;

    // dBias = dOutput
    // dBias = {1, 1}
    isPassed = isPassed &&
        dParameters[8] == 1.0 && dParameters[9] == 1.0;

    return isPassed;
}

void cleanupDParameters()
{
    for (int i = 0; i < 10; i++)
    {
        dParameters[i] = ElementType(0.0);
    }
}

void setupParameters()
{
    for (int i = 0; i < 10; i++)
    {
        parameters[i] = ElementType(parametersFloat[i]);
    }
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    setupParameters();

    testResult[0] = BasicTestWithoutBias();

    cleanupDParameters();

    testResult[1] = BasicTestWithBias();

    // BUFFER: 1
    // BUFFER: 1
}
