//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-vk -compute -shaderobj -xslang -experimental-feature -output-using-type -xslang -DTEST_HALF=0
// //TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-dx12 -compute -shaderobj -profile cs_6_6 -xslang -experimental-feature -output-using-type -xslang -DTEST_HALF=0
// //TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-mtl -compute -shaderobj -output-using-type -xslang -experimental-feature -xslang -DTEST_HALF=0

// Currently, only CUDA supports atomicAdd on half. So we can only test fp16 on CUDA.
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0  -xslang -experimental-feature -xslang -DTEST_HALF=0
// //TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUFFER):-cuda -compute -shaderobj -output-using-type -capability cuda_sm_7_0 -xslang -experimental-feature -xslang -DTEST_HALF=1
import neural;
typealias ElementType = half;

// set up a 2x4 matrix for input parameters, the last 2 elements are for bias
//TEST_INPUT: ubuffer(data=[1.0 2.0 3.0 4.0 5.0 6.0 7.0 8.0 9.0 10.0], stride=4):name=parametersFloat
// 1 2 3 4
// 5 6 7 8
// bias = {9.0, 10.0}
RWStructuredBuffer<float> parametersFloat;

//TEST_INPUT: ubuffer(data=[0 0 0 0 0 0 0 0 0 0], stride=2):name=parameters
RWStructuredBuffer<ElementType> parameters;

// Create a buffer to store the test result
//TEST_INPUT: ubuffer(data=[0 0 0 0], stride=4):out,name=testResult
RWStructuredBuffer<uint2> testResult;

//TEST_INPUT: ubuffer(data=[0 0 0 0], stride=4):name=dInput
RWStructuredBuffer<ElementType> dInput;

// set up a 2x4 matrix for derivative of parameters, the last 2 elements are for derivative of bias
//TEST_INPUT: ubuffer(data=[0 0 0 0 0 0 0 0 0 0], stride=4):name=dParameters
RWStructuredBuffer<ElementType> dParameters;

typealias BufferStorage = StructuredBufferStorage<ElementType>;

static const int InputSize = 4;
static const int OutputSize = 2;
static const int BatchSize = 32;
static const int SubgroupSize = 32;

#define MAX(a, b) (a < b ? a : b)

static const int ShMemSize = ( MMAHelper<ElementType, InputSize, OutputSize, SubgroupSize>.SharedMemSizeInVectorMatA +
                               MMAHelper<ElementType, InputSize, OutputSize, SubgroupSize>.SharedMemSizeInVectorMatB);

groupshared uint4[ShMemSize] shMem;
typealias SPtr<T> = Ptr<T, Access::ReadWrite, AddressSpace::GroupShared>;

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

// [Differentiable]
// OutputVector TestInlineVectorMatMulAdd<InputVector, OutputVector>(
//     InputVector input,
//     BufferStorage weightStorage,
//     BufferStorage biasStorage,
//     BufferStorage.Address weightAddress,
//     BufferStorage.Address biasAddress)
//     where InputVector : IVector<ElementType, 4>
//     where OutputVector : IVector<ElementType, 2>
// {
//     var outputVec = input.linearTransform<2, BufferStorage, OutputVector>(weightStorage, biasStorage, weightAddress, biasAddress);
//     return outputVec;
// }

// Basic test on MatMul without bias, this test covers both forward and backward pass
void BasicTestWithoutBias(int tid, int resIndex)
{
    typealias InVectorType = AccelerateVectorCoopMat<ElementType, 4, SubgroupSize>;
    typealias OutVectorType = AccelerateVectorCoopMat<ElementType, 2, SubgroupSize>;

    ElementType[4] inputData = {ElementType(1.0), ElementType(2.0), ElementType(3.0), ElementType(4.0)};
    var input = InVectorType(inputData);
    input.sharedMemoryPtr = __getAddress(shMem[0]);

    BufferStorage weightStorage = BufferStorage(parameters);
    BufferStorage dweightStorage = BufferStorage(dParameters);
    BufferStorage.Address weightAddress = 0;

    // Run the forward pass
    let outputVec = TestInlineVectorMatMul<InVectorType, OutVectorType>(input, weightStorage, weightAddress);
    // serialRead(tid, (input.sharedMemoryPtr + 32));

    // (1*1 + 2*2 + 3*3 + 4*4) = 30.0
    // (5*1 + 6*2 + 7*3 + 8*4) = 70.0
    bool isPassed = (outputVec[0] == 30.0 && outputVec[1] == 70.0);
    isPassed = WaveActiveAllTrue(isPassed);
    if (tid == 0)
    {
        testResult[resIndex].x = isPassed ? 1 : 0;
    }


    var weightDiffPair = DifferentialPtrPair<BufferStorage>(weightStorage, dweightStorage);
    let dRes = OutVectorType(1.0f);
    var dPair = diffPair(input);

    // Run the backward pass
    // dInput = W^T * dOutput
    // dInput = {3, 7, 11, 15}
    bwd_diff(TestInlineVectorMatMul<InVectorType, OutVectorType>)
                (dPair, weightDiffPair, weightAddress, dRes);

    if (tid == 0)
    {
        for (int i = 0; i < 4; i++)
        {
            printf("%.1f ", float(dPair.d[i]));
        }
        printf("\n");
    }

    // isPassed = isPassed &&
    //     dPair.d[0] == 3.0 && dPair.d[1] == 7.0 && dPair.d[2] == 11.0 && dPair.d[3] == 15.0;

    // // dW = dOutput * dInput^T
    // // dW = [1, 1]^T * [1, 2, 3, 4]
    // //    = [[1, 2, 3, 4]; [1, 2, 3, 4]]
    // isPassed = isPassed &&
    //     dParameters[0] == 1.0 && dParameters[1] == 2.0 && dParameters[2] == 3.0 && dParameters[3] == 4.0 &&
    //     dParameters[4] == 1.0 && dParameters[5] == 2.0 && dParameters[6] == 3.0 && dParameters[7] == 4.0;
}

void serialRead(uint tid, SPtr<uint4> sharedMem)
{
    GroupMemoryBarrierWithWaveSync();

    if (tid > 0)
        return;

    for (int id = 0; id < 16; id++)
    {
        printf("tid: %d\n", id);
        for (int i = 0; i < 2; i++)
        {
            uint4 values = sharedMem[id * 2 + i];
            uint4 element = values;
            for (int j = 0; j < 4; j++)
            {
                uint value = element[j];
                uint16_t a = (uint16_t)(value & 0xFFFF);
                uint16_t b = (uint16_t)((value >> 16) & 0xFFFF);

                half aa = bit_cast<half>(a);
                half bb = bit_cast<half>(b);
                printf("%.1f %.1f ", float(aa), float(bb));
            }
        }
        printf("\n");
    }
}

void cleanupDParameters()
{
    for (int i = 0; i < 10; i++)
    {
        dParameters[i] = ElementType(0.0);
    }
}

void setupParameters(uint tid)
{
    if (tid == 0)
    {
        for (int i = 0; i < 10; i++)
        {
            parameters[i] = ElementType(parametersFloat[i]);
        }
    }
}

[shader("compute")]
[numthreads(BatchSize, 1, 1)]
void computeMain(uint tid : SV_DispatchThreadID)
{
    setupParameters(tid);
    GroupMemoryBarrierWithWaveSync();
    BasicTestWithoutBias(tid, 0);
    // BUFFER: 1
}
