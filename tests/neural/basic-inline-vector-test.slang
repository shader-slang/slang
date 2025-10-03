//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-slang -compute -shaderobj -output-using-type


import neural;

// set up a 2x4 matrix for input parameters
//TEST_INPUT: ubuffer(data=[1.0 2.0 3.0 4.0 5.0 6.0 7.0 8.0], stride=4):name=parameters
// 1 2 3 4
// 5 6 7 8
RWStructuredBuffer<float> parameters;


// set up a 2x1 matrix for output
//TEST_INPUT: ubuffer(data=[0 0], stride=4):out,name=output
RWStructuredBuffer<float> output;


//TEST_INPUT: ubuffer(data=[0 0 0 0], stride=4):out,name=dout_input
RWStructuredBuffer<float> dout_input;

//TEST_INPUT: ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=dparameters
RWStructuredBuffer<float> dparameters;

typealias BufferStorage = StructuredBufferStorage<float>;

OutputVector testForward<InputVector, OutputVector>(
    InputVector input)
    where InputVector : IVector<float, 4>
    where OutputVector : IVector<float, 2>
{
    BufferStorage storage = BufferStorage(parameters);
    BufferStorage.Address address = 0;

    var outputVec = input.eval<2, false, BufferStorage, OutputVector>(storage, address);
    return outputVec;
}

[Differentiable]
OutputVector testBackward<InputVector, OutputVector>(
    InputVector input,
    BufferStorage storage,
    BufferStorage.Address address)
    where InputVector : IVector<float, 4>
    where OutputVector : IVector<float, 2>
{
    var outputVec = input.eval<2, false, BufferStorage, OutputVector>(storage, address);
    return outputVec;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    float[4] inputData = {1.0f, 2.0f, 3.0f, 4.0f};
    let input = InlineVector<float, 4>(inputData);

    let outputVec = testForward<InlineVector<float, 4>, InlineVector<float, 2>>(input);
    
    // BUFFER: 50.0
    // BUFFER: 60.0
    // (1*1 + 2*3 + 3*5 + 4*7) = 50.0
    // (1*2 + 2*4 + 3*6 + 4*8) = 60.0
    output[0] = outputVec[0];
    output[1] = outputVec[1];

    BufferStorage storage = BufferStorage(parameters);
    BufferStorage dstorage = BufferStorage(dparameters);
    BufferStorage.Address address = 0;

    var parameterDiffPair = DifferentialPtrPair<BufferStorage>(storage, dstorage);

    let dRes = InlineVector<float, 2>(1.0f);
    var dPair = diffPair(input);

    // dInput = W^T * dOutput
    // dInput = {3, 7, 11, 15}
    bwd_diff(testBackward<InlineVector<float, 4>, InlineVector<float, 2>>)(dPair, parameterDiffPair, address, dRes);
    // BUFFER: 3.0
    // BUFFER: 7.0
    // BUFFER: 11.0
    // BUFFER: 15.0
    dout_input[0] = dPair.d[0];
    dout_input[1] = dPair.d[1];
    dout_input[2] = dPair.d[2];
    dout_input[3] = dPair.d[3];

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
