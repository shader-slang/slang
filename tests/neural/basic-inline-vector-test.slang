//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -output-using-type -xslang -DTEST_BINDLESS_STORAGE=0
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -xslang -DTEST_BINDLESS_STORAGE=0
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-dx12 -compute -shaderobj -profile cs_6_6 -output-using-type -xslang -DTEST_BINDLESS_STORAGE=0
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-mtl -compute -shaderobj -output-using-type -xslang -DTEST_BINDLESS_STORAGE=0


import neural;

// set up a 2x4 matrix for input parameters
//TEST_INPUT: ubuffer(data=[1.0 2.0 3.0 4.0 5.0 6.0 7.0 8.0], stride=4):name=parameters
// 1 2 3 4
// 5 6 7 8
RWStructuredBuffer<float> parameters;

// set up a 2x1 matrix for output
//TEST_INPUT: ubuffer(data=[0 0], stride=4):out,name=output
RWStructuredBuffer<float> output;

//TEST_INPUT: ubuffer(data=[0 0 0 0], stride=4):out,name=dInput
RWStructuredBuffer<float> dInput;

//TEST_INPUT: ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=dParameters
RWStructuredBuffer<float> dParameters;

typealias BufferStorage = StructuredBufferStorage<float>;
typealias BindlessStorage = BindlessBufferStorage<float>;

RWStructuredBuffer<float>.Handle parametersHandle;
RWStructuredBuffer<float>.Handle dParametersHandle;

// set up a 2x1 matrix for output
//TEST_INPUT: ubuffer(data=[0 0], stride=4):out,name=output2
RWStructuredBuffer<float> output2;

[Differentiable]
OutputVector TestInlineVectorMatMulAdd<InputVector, OutputVector>(
    InputVector input,
    BufferStorage storage,
    BufferStorage.Address address)
    where InputVector : IVector<float, 4>
    where OutputVector : IVector<float, 2>
{
    var outputVec = input.eval<2, false, BufferStorage, OutputVector>(storage, address);
    return outputVec;
}

OutputVector TestInlineVectorMatMulAddForBindlessStorage<InputVector, OutputVector>(
    InputVector input,
    BindlessStorage.Address address)
    where InputVector : IVector<float, 4>
    where OutputVector : IVector<float, 2>
{
    var outputVec = input.eval<2, BindlessStorage, OutputVector>(address);
    return outputVec;
}

void BasicTest()
{
    float[4] inputData = {1.0f, 2.0f, 3.0f, 4.0f};
    let input = InlineVector<float, 4>(inputData);
    BufferStorage storage = BufferStorage(parameters);
    BufferStorage dstorage = BufferStorage(dParameters);
    BufferStorage.Address address = 0;

    // Run the forward pass
    let outputVec = TestInlineVectorMatMulAdd<InlineVector<float, 4>, InlineVector<float, 2>>(input, storage, address);

    // BUFFER: 50.0
    // BUFFER: 60.0
    // (1*1 + 2*3 + 3*5 + 4*7) = 50.0
    // (1*2 + 2*4 + 3*6 + 4*8) = 60.0
    output[0] = outputVec[0];
    output[1] = outputVec[1];

    var parameterDiffPair = DifferentialPtrPair<BufferStorage>(storage, dstorage);
    let dRes = InlineVector<float, 2>(1.0f);
    var dPair = diffPair(input);

    // Run the backward pass
    // dInput = W^T * dOutput
    // dInput = {3, 7, 11, 15}
    bwd_diff(TestInlineVectorMatMulAdd<InlineVector<float, 4>, InlineVector<float, 2>>)
                (dPair, parameterDiffPair, address, dRes);
    // BUFFER: 3.0
    // BUFFER: 7.0
    // BUFFER: 11.0
    // BUFFER: 15.0
    dInput[0] = dPair.d[0];
    dInput[1] = dPair.d[1];
    dInput[2] = dPair.d[2];
    dInput[3] = dPair.d[3];

    // dW = dOutput * dInput^T
    // dW = [1, 1]^T * [1, 2, 3, 4]    // dW = {5, 11, 17, 23}
    // BUFFER: 1.0
    // BUFFER: 1.0
    // BUFFER: 2.0
    // BUFFER: 2.0
    // BUFFER: 3.0
    // BUFFER: 3.0
    // BUFFER: 4.0
    // BUFFER: 4.0
}

// TODO: Currently, our test framework doesn't support bindless storage setup. So we have to
// disable the bindless storage test for now.
#if TEST_BINDLESS_STORAGE
void BasicTestForBindlessStorage()
{
    float[4] inputData = {1.0f, 2.0f, 3.0f, 4.0f};
    let input = InlineVector<float, 4>(inputData);
    BindlessStorage.Address parametersAddress = BindlessStorage.Address(parametersHandle);

    for (int i = 0; i < 8; i++)
    {
        parametersAddress[i] = i + 1.0f;
    }

    // Run the forward pass
    let outputVec = TestInlineVectorMatMulAddForBindlessStorage<InlineVector<float, 4>, InlineVector<float, 2>>
                        (input, parametersAddress);

    output2[0] = outputVec[0];
    output2[1] = outputVec[1];
}
#endif


[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    BasicTest();
#if TEST_BINDLESS_STORAGE
    BasicTestForBindlessStorage();
#endif
}
