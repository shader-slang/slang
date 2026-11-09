// This is a self-contained reduction of an NVIDIA Vulkan driver miscompile observed on an
// RTX 5090 with driver 610.88. Before the cross-subgroup loop was statically unrolled, both
// optimization levels compiled valid SPIR-V from the same Slang IR but produced different gradient
// checksums on the affected driver:
//
//   slangi -O0 -api vulkan tests/bugs/spirv-o0-neural-gradient-miscompile.slang
//   5.902252
//   error: O0 produced incorrect neural gradients
//
//   slangi -O1 -api vulkan tests/bugs/spirv-o0-neural-gradient-miscompile.slang
//   9.272705
//
// O0 leaves constant, case-less OpSwitch instructions in the two-subgroup cooperative-matrix
// reduction. O1 removes those dead branches. The O0 module passes spirv-val for Vulkan 1.3.
// The neural helper now statically unrolls this compile-time-sized reduction. The runtime main
// checks the mixed-precision address's new FP32-gradient checksum. It is not an enabled test
// because it requires cooperative matrices and bindless buffers.
//
//TEST:SIMPLE(filecheck=SPIRV): -target spirv -entry accumulateGradients -stage compute -emit-spirv-directly -O0 -experimental-feature
// SPIRV: OpCapability CooperativeMatrixKHR
// SPIRV: OpCapability AtomicFloat32AddEXT
// SPIRV-NOT: AtomicFloat16AddEXT
// SPIRV-NOT: AtomicFloat16VectorNV
// SPIRV: OpAtomicFAddEXT %float
// SPIRV: OpControlBarrier

import slang.neural;

static const int kInputSize = 1;
static const int kHiddenSize = 19;
static const int kOutputSize = 16;
static const int kSubgroupSize = 32;
static const int kSubgroupCount = 2;
static const uint kThreadCount = kSubgroupSize * kSubgroupCount;

typealias ReproSharedMemory = SharedMemorySize<
    half,
    TargetEnum.SPIR_V,
    ExecutionMode.Training,
    kSubgroupSize,
    kSubgroupCount,
    kInputSize,
    kHiddenSize,
    kOutputSize>;
typealias ReproParameterAddress = BindlessAddressQ<half, float>;

struct ReproNetwork
{
    typealias InputVector = WaveTangledVector<
        half, ReproSharedMemory, kInputSize, kSubgroupSize, kSubgroupCount>;
    typealias HiddenVector = WaveTangledVector<
        half, ReproSharedMemory, kHiddenSize, kSubgroupSize, kSubgroupCount>;
    typealias OutputVector = WaveTangledVector<
        half, ReproSharedMemory, kOutputSize, kSubgroupSize, kSubgroupCount>;
    typealias InputLayer = FFLayer<
        half, InputVector, HiddenVector, LinearLayout, ReLU<half>, false>;
    typealias OutputLayer = FFLayer<
        half, HiddenVector, OutputVector, LinearLayout, IdentityActivation<half>, false>;

    static const uint storageParameterCount =
        InputLayer::StorageSize + OutputLayer::StorageSize;

    RWStructuredBuffer<half>.Handle parameters;

    __init(RWStructuredBuffer<half>.Handle parameters)
    {
        this.parameters = parameters;
    }

    [Differentiable]
    [NoDiffThis]
    OutputVector evalWithParameters<A>(InputVector input, A parameterAddress)
        where A : IPointerLikeAddress<half>
        where A.Differential : IPointerLikeAddress<half>
    {
        InputLayer inputLayer;
        HiddenVector hidden = inputLayer.eval<A>(input, parameterAddress);
        OutputLayer outputLayer;
        return outputLayer.eval<A>(
            hidden,
            parameterAddress.getOffset(InputLayer::StorageSize));
    }
}

[Differentiable]
half networkLoss(
    no_diff ReproNetwork network,
    no_diff uint sampleIndex,
    ReproParameterAddress parameterAddress)
{
    ReproNetwork::InputVector input;
    for (uint i = 0; i < kInputSize; ++i)
    {
        int inputValue = int((sampleIndex * 7 + i * 3) % 17) - 8;
        input[i] = half(inputValue) * 0.03125h;
    }

    ReproNetwork::OutputVector output =
        network.evalWithParameters<ReproParameterAddress>(
            input,
            parameterAddress);

    half loss = 0.0h;
    for (uint i = 0; i < 1; ++i)
    {
        half difference = output[i] - half(i + 1) * 0.125h;
        loss += 0.5h * difference * difference;
    }
    return loss;
}

[shader("compute")]
[numthreads(kThreadCount, 1, 1)]
void initializeParameters(
    uint parameterIndex : SV_DispatchThreadID,
    uniform RWStructuredBuffer<half>.Handle parameters)
{
    if (parameterIndex >= ReproNetwork::storageParameterCount)
        return;

    int value = int((parameterIndex * 13 + 5) % 29) - 14;
    parameters[parameterIndex] = half(value) * 0.00390625h;
}

[shader("compute")]
[numthreads(kThreadCount, 1, 1)]
void clearGradients(
    uint parameterIndex : SV_DispatchThreadID,
    uniform RWStructuredBuffer<float>.Handle gradients)
{
    if (parameterIndex < ReproNetwork::storageParameterCount)
        gradients[parameterIndex] = 0.0;
}

[shader("compute")]
[numthreads(kThreadCount, 1, 1)]
void accumulateGradients(
    uint sampleIndex : SV_GroupIndex,
    uniform ReproNetwork network,
    uniform RWStructuredBuffer<float>.Handle gradient)
{
    DifferentialPtrPair<ReproParameterAddress> parameters =
        DifferentialPtrPair<ReproParameterAddress>(
            ReproParameterAddress(network.parameters),
            ReproParameterAddress::makeDifferential(gradient));
    bwd_diff(networkLoss)(network, sampleIndex, parameters, 1.0h);
}

[shader("compute")]
[numthreads(kThreadCount, 1, 1)]
void checksumGradients(
    uint parameterIndex : SV_DispatchThreadID,
    uniform RWStructuredBuffer<float>.Handle gradient,
    RWStructuredBuffer<float> checksum)
{
    if (parameterIndex >= ReproNetwork::storageParameterCount)
        return;

    float value = abs(gradient[parameterIndex]);
    float subgroupValue = WaveActiveSum(value);
    if (WaveIsFirstLane())
        InterlockedAdd(checksum[0], subgroupValue);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void clearChecksum(RWStructuredBuffer<float> checksum)
{
    checksum[0] = 0.0f;
}

[host]
void main()
{
    RWStructuredBuffer<half> parameters =
        RWStructuredBuffer<half>::alloc(ReproNetwork::storageParameterCount);
    RWStructuredBuffer<float> gradient =
        RWStructuredBuffer<float>::alloc(ReproNetwork::storageParameterCount);

    dispatch(
        initializeParameters,
        uint3(ReproNetwork::storageParameterCount, 1, 1))(
        parameters.getDescriptorHandle());
    dispatch(
        clearGradients,
        uint3(ReproNetwork::storageParameterCount, 1, 1))(
        gradient.getDescriptorHandle());

    ReproNetwork network = ReproNetwork(parameters.getDescriptorHandle());
    dispatch(accumulateGradients, uint3(kThreadCount, 1, 1))(
        network,
        gradient.getDescriptorHandle());

    RWStructuredBuffer<float> checksumBuffer = RWStructuredBuffer<float>::alloc(1);
    dispatch(clearChecksum, uint3(1, 1, 1))(checksumBuffer);
    dispatch(
        checksumGradients,
        uint3(ReproNetwork::storageParameterCount, 1, 1))(
        gradient.getDescriptorHandle(),
        checksumBuffer);

    float checksum = asyncReadBack(checksumBuffer, 0).wait();
    print(checksum);
    if (checksum < 5.90f || checksum > 5.91f)
    {
        String message = "mixed-precision neural gradient checksum is incorrect";
        hgbc_asm
        {
            OpFail $message;
        };
    }
}
