// This test is to test InlineVector with Bindless Storage.

// Both Pointer and DescriptorHandle are belong to this category. However, since there is issue on our test-infrasture on Vulkan for
// the DescriptorHandle support, we can only test Pointer for Vulkan now. See issue #8631.

// We don't want to enable the pointer test on glsl, because there is a bug in emitting glsl code. See issue #8630.

// Currently, only CUDA supports atomicAdd on half. So we can only test fp16 on CUDA.


//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0 -xslang -experimental-feature -xslang -DTEST_HALF=1 -xslang -DTEST_POINTER=1
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0 -xslang -experimental-feature -xslang -DTEST_HALF=1 -xslang -DTEST_POINTER=0

//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature -xslang -DTEST_HALF=0  -xslang -DTEST_POINTER=1
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature -xslang -DTEST_HALF=0 -xslang -DTEST_POINTER=0

//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -output-using-type -emit-spirv-directly -xslang -experimental-feature -xslang -DTEST_HALF=0 -xslang -DTEST_POINTER=1

//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-mtl -compute -shaderobj -output-using-type -xslang -experimental-feature -xslang -DTEST_HALF=0 -xslang -DTEST_POINTER=1
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-mtl -compute -shaderobj -output-using-type -xslang -experimental-feature -xslang -DTEST_HALF=0 -xslang -DTEST_POINTER=0

import neural;

#if TEST_HALF
typealias ElementType = half;
#else
typealias ElementType = float;
#endif

// Create a buffer to store the test result
//TEST_INPUT: ubuffer(data=[0 0], stride=4):out,name=testResult
RWStructuredBuffer<uint> testResult;


// set up a 2x4 matrix for input parameters, the last 2 elements are for bias
//TEST_INPUT: set parametersFloat = ubuffer(data=[1.0 2.0 3.0 4.0 5.0 6.0 7.0 8.0 9.0 10.0], stride=4)
// 1 2 3 4
// 5 6 7 8
// bias = {9.0, 10.0}
uniform RWStructuredBuffer<float> parametersFloat;

//TEST_INPUT: set parameters = ubuffer(data=[0 0 0 0 0 0 0 0 0 0], stride=2)
//TEST_INPUT: ubuffer(data=[0 0 0 0], stride=4):name=dInput

// set up a 2x4 matrix for derivative of parameters, the last 2 elements are for derivative of bias
//TEST_INPUT: set dParameters = ubuffer(data=[0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0 0.0], stride=4)

#if TEST_POINTER
    uniform ElementType* parameters;
    uniform ElementType* dInput;
    uniform ElementType* dParameters;
    typealias Address = PointerAddress<ElementType>;
#else
    uniform RWStructuredBuffer<ElementType>.Handle parameters;
    uniform RWStructuredBuffer<ElementType>.Handle dInput;
    uniform RWStructuredBuffer<ElementType>.Handle dParameters;
    typealias Address = BindlessAddress<ElementType>;
#endif


[Differentiable]
OutputVector TestInlineVectorMatMul<InputVector, OutputVector>(
    InputVector input,
    Address address)
    where InputVector : IVector<ElementType, 4>
    where OutputVector : IVector<ElementType, 2>
{
    var outputVec = input.linearTransform<2, Address, OutputVector>(address);
    return outputVec;
}

[Differentiable]
OutputVector TestInlineVectorMatMulAdd<InputVector, OutputVector>(
    InputVector input,
    Address weightAddress,
    Address biasAddress)
    where InputVector : IVector<ElementType, 4>
    where OutputVector : IVector<ElementType, 2>
{
    var outputVec = input.linearTransform<2, Address, OutputVector>(weightAddress, biasAddress);
    return outputVec;
}

// Basic test on MatMul without bias, this test covers both forward and backward pass
bool BasicTestWithoutBias()
{
    ElementType[4] inputData = {ElementType(1.0), ElementType(2.0), ElementType(3.0), ElementType(4.0)};
    let input = InlineVector<ElementType, 4>(inputData);
    Address weightAddress = { parameters };
    Address dWeightAddress = { dParameters };

    // Run the forward pass
    let outputVec = TestInlineVectorMatMul<InlineVector<ElementType, 4>, InlineVector<ElementType, 2>>(input, weightAddress);

    // (1*1 + 2*2 + 3*3 + 4*4) = 30.0
    // (5*1 + 6*2 + 7*3 + 8*4) = 70.0
    bool isPassed = (outputVec[0] == 30.0 && outputVec[1] == 70.0);

    var weightDiffPair = DifferentialPtrPair<Address>(weightAddress, dWeightAddress);
    let dRes = InlineVector<ElementType, 2>(1.0f);
    var dPair = diffPair(input);

    // Run the backward pass
    // dInput = W^T * dOutput
    // dInput = {3, 7, 11, 15}
    bwd_diff(TestInlineVectorMatMul<InlineVector<ElementType, 4>, InlineVector<ElementType, 2>>)
                (dPair, weightDiffPair, dRes);

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
    Address weightAddress = Address(parameters);
    Address biasAddress = weightAddress.getOffset(8);
    Address dWeightAddress = Address(dParameters);
    Address dBiasAddress = dWeightAddress.getOffset(8);

    // Run the forward pass
    let outputVec = TestInlineVectorMatMulAdd<InlineVector<ElementType, 4>, InlineVector<ElementType, 2>>(input, weightAddress, biasAddress);

    // (1*1 + 2*2 + 3*3 + 4*4) + 9.0 = 39.0
    // (5*1 + 6*2 + 7*3 + 8*4) + 10.0 = 80.0
    bool isPassed = (outputVec[0] == 39.0 && outputVec[1] == 80.0);

    var weightDiffPair = DifferentialPtrPair<Address>(weightAddress, dWeightAddress);
    var biasDiffPair = DifferentialPtrPair<Address>(biasAddress, dBiasAddress);
    let dOutput = InlineVector<ElementType, 2>(1.0);
    var dPair = diffPair(input);

    // Run the backward pass
    // dInput = W^T * dOutput
    // dInput = {3, 7, 11, 15}
    bwd_diff(TestInlineVectorMatMulAdd<InlineVector<ElementType, 4>, InlineVector<ElementType, 2>>)
                (dPair, weightDiffPair, biasDiffPair, dOutput);

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
