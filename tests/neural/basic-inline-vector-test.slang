//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-dx12 -compute -shaderobj -profile cs_6_6 -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-mtl -compute -shaderobj -output-using-type
import neural;

// set up a 2x4 matrix for input parameters, the last 2 elements are for bias
//TEST_INPUT: ubuffer(data=[1.0 2.0 3.0 4.0 5.0 6.0 7.0 8.0 9.0 10.0], stride=4):name=parameters
// 1 2 3 4
// 5 6 7 8
// bias = {9.0, 10.0}
RWStructuredBuffer<float> parameters;

// Create a buffer to store the test result
//TEST_INPUT: ubuffer(data=[0 0], stride=4):out,name=testResult
RWStructuredBuffer<uint> testResult;

//TEST_INPUT: ubuffer(data=[0 0 0 0], stride=4):name=dInput
RWStructuredBuffer<float> dInput;

// set up a 2x4 matrix for derivative of parameters, the last 2 elements are for derivative of bias
//TEST_INPUT: ubuffer(data=[0 0 0 0 0 0 0 0 0 0], stride=4):name=dParameters
RWStructuredBuffer<float> dParameters;

typealias BufferStorage = StructuredBufferStorage<float>;
typealias BindlessStorage = BindlessBufferStorage<float>;

[Differentiable]
OutputVector TestInlineVectorMatMulAdd<InputVector, OutputVector, bool Bias>(
    InputVector input,
    BufferStorage storage,
    BufferStorage.Address address)
    where InputVector : IVector<float, 4>
    where OutputVector : IVector<float, 2>
{
    var outputVec = input.matmul<2, Bias, BufferStorage, OutputVector>(storage, address);
    return outputVec;
}

OutputVector TestInlineVectorMatMulAddForBindlessStorage<InputVector, OutputVector, bool Bias>(
    InputVector input,
    BindlessStorage.Address address)
    where InputVector : IVector<float, 4>
    where OutputVector : IVector<float, 2>
{
    var outputVec = input.matmul<2, Bias, BindlessStorage, OutputVector>(address);
    return outputVec;
}

// Basic test on MatMul without bias, this test covers both forward and backward pass
bool BasicTest()
{
    float[4] inputData = {1.0f, 2.0f, 3.0f, 4.0f};
    let input = InlineVector<float, 4>(inputData);
    BufferStorage storage = BufferStorage(parameters);
    BufferStorage dstorage = BufferStorage(dParameters);
    BufferStorage.Address address = 0;

    // Run the forward pass
    let outputVec = TestInlineVectorMatMulAdd<InlineVector<float, 4>, InlineVector<float, 2>, false>(input, storage, address);

    // (1*1 + 2*2 + 3*3 + 4*4) = 30.0
    // (5*1 + 6*2 + 7*3 + 8*4) = 70.0
    bool isPassed = (outputVec[0] == 30.0f && outputVec[1] == 70.0f);

    var parameterDiffPair = DifferentialPtrPair<BufferStorage>(storage, dstorage);
    let dRes = InlineVector<float, 2>(1.0f);
    var dPair = diffPair(input);

    // Run the backward pass
    // dInput = W^T * dOutput
    // dInput = {3, 7, 11, 15}
    bwd_diff(TestInlineVectorMatMulAdd<InlineVector<float, 4>, InlineVector<float, 2>, false>)
                (dPair, parameterDiffPair, address, dRes);

    isPassed = isPassed &&
        dPair.d[0] == 3.0f && dPair.d[1] == 7.0f && dPair.d[2] == 11.0f && dPair.d[3] == 15.0f;

    // dW = dOutput * dInput^T
    // dW = [1, 1]^T * [1, 2, 3, 4]
    //    = [[1, 2, 3, 4]; [1, 2, 3, 4]]
    isPassed = isPassed &&
        dParameters[0] == 1.0f && dParameters[1] == 2.0f && dParameters[2] == 3.0f && dParameters[3] == 4.0f &&
        dParameters[4] == 1.0f && dParameters[5] == 2.0f && dParameters[6] == 3.0f && dParameters[7] == 4.0f;

    return isPassed;
}

// Basic test on MatMul with bias, this test covers both forward and backward pass
bool BasicTestWithBias()
{
    float[4] inputData = {1.0f, 2.0f, 3.0f, 4.0f};
    let input = InlineVector<float, 4>(inputData);
    BufferStorage storage = BufferStorage(parameters);
    BufferStorage dstorage = BufferStorage(dParameters);
    BufferStorage.Address address = 0;

    // Run the forward pass
    let outputVec = TestInlineVectorMatMulAdd<InlineVector<float, 4>, InlineVector<float, 2>, true>(input, storage, address);

    // (1*1 + 2*2 + 3*3 + 4*4) + 9.0 = 39.0
    // (5*1 + 6*2 + 7*3 + 8*4) + 10.0 = 80.0
    bool isPassed = (outputVec[0] == 39.0f && outputVec[1] == 80.0f);

    var parameterDiffPair = DifferentialPtrPair<BufferStorage>(storage, dstorage);
    let dOutput = InlineVector<float, 2>(1.0f);
    var dPair = diffPair(input);

    // Run the backward pass
    // dInput = W^T * dOutput
    // dInput = {3, 7, 11, 15}
    bwd_diff(TestInlineVectorMatMulAdd<InlineVector<float, 4>, InlineVector<float, 2>, true>)
                (dPair, parameterDiffPair, address, dOutput);

    isPassed = isPassed &&
        dPair.d[0] == 3.0f && dPair.d[1] == 7.0f && dPair.d[2] == 11.0f && dPair.d[3] == 15.0f;

    // dW = dOutput * dInput^T
    // dW = [1, 1]^T * [1, 2, 3, 4]
    //    = [[1, 2, 3, 4]; [1, 2, 3, 4]]
    isPassed = isPassed &&
        dParameters[0] == 1.0f && dParameters[1] == 2.0f && dParameters[2] == 3.0f && dParameters[3] == 4.0f &&
        dParameters[4] == 1.0f && dParameters[5] == 2.0f && dParameters[6] == 3.0f && dParameters[7] == 4.0f;

    // dBias = dOutput
    // dBias = {1, 1}
    isPassed = isPassed &&
        dParameters[8] == 1.0f && dParameters[9] == 1.0f;

    return isPassed;
}

void cleanupDParameters()
{
    for (int i = 0; i < 10; i++)
    {
        dParameters[i] = 0.0f;
    }
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    testResult[0] = BasicTest();
    cleanupDParameters();
    testResult[1] = BasicTestWithBias();

    // BUFFER: 1
    // BUFFER: 1
}
