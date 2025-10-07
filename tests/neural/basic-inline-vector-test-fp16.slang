//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -output-using-type

import neural;

// set up a 2x4 matrix for input parameters, the last 2 elements are for bias
//TEST_INPUT: ubuffer(data=[1.0 2.0 3.0 4.0 5.0 6.0 7.0 8.0 9.0 10.0], stride=4):name=parametersFloat
// 1 2 3 4
// 5 6 7 8
// bias = {9.0, 10.0}
RWStructuredBuffer<float> parametersFloat;

//TEST_INPUT: ubuffer(data=[0 0 0 0 0 0 0 0 0 0], stride=2):name=parameters
RWStructuredBuffer<half> parameters;


// Create a buffer to store the test result
//TEST_INPUT: ubuffer(data=[0 0], stride=4):out,name=testResult
RWStructuredBuffer<float> testResult;

//TEST_INPUT: ubuffer(data=[0 0 0 0], stride=2):name=dInput
RWStructuredBuffer<half> dInput;

// set up a 2x4 matrix for derivative of parameters, the last 2 elements are for derivative of bias
//TEST_INPUT: ubuffer(data=[0 0 0 0 0 0 0 0 0 0], stride=2):name=dParameters
RWStructuredBuffer<half> dParameters;

typealias BufferStorage = StructuredBufferStorage<half>;
typealias BindlessStorage = BindlessBufferStorage<half>;

[Differentiable]
OutputVector TestInlineVectorMatMulAdd<InputVector, OutputVector, bool Bias>(
    InputVector input,
    BufferStorage storage,
    BufferStorage.Address address)
    where InputVector : IVector<half, 4>
    where OutputVector : IVector<half, 2>
{
    var outputVec = input.matmul<2, Bias, BufferStorage, OutputVector>(storage, address);
    return outputVec;
}

OutputVector TestInlineVectorMatMulAddForBindlessStorage<InputVector, OutputVector, bool Bias>(
    InputVector input,
    BindlessStorage.Address address)
    where InputVector : IVector<half, 4>
    where OutputVector : IVector<half, 2>
{
    var outputVec = input.matmul<2, Bias, BindlessStorage, OutputVector>(address);
    return outputVec;
}

// Basic test on MatMul with bias, this test covers both forward and backward pass
bool BasicTest()
{
    half[4] inputData = {1.0h, 2.0h, 3.0h, 4.0h};
    let input = InlineVector<half, 4>(inputData);
    BufferStorage storage = BufferStorage(parameters);
    BufferStorage dstorage = BufferStorage(dParameters);
    BufferStorage.Address address = 0;

    // Run the forward pass
    let outputVec = TestInlineVectorMatMulAdd<InlineVector<half, 4>, InlineVector<half, 2>, true>(input, storage, address);

    // (1*1 + 2*2 + 3*3 + 4*4) + 9.0 = 39.0
    // (5*1 + 6*2 + 7*3 + 8*4) + 10.0 = 80.0
    bool isPassed = (outputVec[0] == 39.0h && outputVec[1] == 80.0h);

    var parameterDiffPair = DifferentialPtrPair<BufferStorage>(storage, dstorage);
    let dRes = InlineVector<half, 2>(1.0h);
    var dPair = diffPair(input);

    // Run the backward pass
    // dInput = W^T * dOutput
    // dInput = {3, 7, 11, 15}
    bwd_diff(TestInlineVectorMatMulAdd<InlineVector<half, 4>, InlineVector<half, 2>, false>)
                (dPair, parameterDiffPair, address, dRes);

    isPassed = isPassed &&
        dPair.d[0] == 3.0h && dPair.d[1] == 7.0h && dPair.d[2] == 11.0h && dPair.d[3] == 15.0h;

    return isPassed;
}

void cleanupDParameters()
{
    for (int i = 0; i < 10; i++)
    {
        dParameters[i] = 0.0h;
    }
}
void setupParameters()
{
    for (int i = 0; i < 10; i++)
    {
        parameters[i] = half(parametersFloat[i]);
    }
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    setupParameters();
    testResult[0] = BasicTest();

    // BUFFER: 1
}
