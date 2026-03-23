import slang.neural;

// Minimal repro WaveTangled MLP:
// 2 -> 128 (LeakyReLU) -> 128 (LeakyReLU) -> 128 (LeakyReLU) -> 3 (Sigmoid)
static const int IN_DIM = 2;
static const int HIDDEN_DIM = 128;
static const int OUT_DIM = 3;

#ifndef WT_WAVE_WARPS
#define WT_WAVE_WARPS 4
#endif

static const int SubgroupSize = 32;
static const int WARPS_PER_BLOCK = WT_WAVE_WARPS;
static const float LEAKY_ALPHA = 0.01f;

typealias ShMemSize = SharedMemorySize<float, TargetEnum.CUDA, ExecutionMode.Training, SubgroupSize, WARPS_PER_BLOCK>;
typealias ShMemSizeLayer = ShMemSize.OfLayer4<IN_DIM, HIDDEN_DIM, HIDDEN_DIM, HIDDEN_DIM, OUT_DIM>;
typealias Storage = TorchTensorViewAddress<float>;

typealias InVec = WaveTangledVector<float, ShMemSizeLayer, IN_DIM, SubgroupSize>;
typealias HidVec = WaveTangledVector<float, ShMemSizeLayer, HIDDEN_DIM, SubgroupSize>;
typealias OutVec = WaveTangledVector<float, ShMemSizeLayer, OUT_DIM, SubgroupSize>;

typealias Layer1 = FFLayer<float, InVec, HidVec, LinearLayout, LeakyReLU<float>, true>;
typealias Layer2 = FFLayer<float, HidVec, HidVec, LinearLayout, LeakyReLU<float>, true>;
typealias Layer3 = FFLayer<float, HidVec, HidVec, LinearLayout, LeakyReLU<float>, true>;
typealias Layer4 = FFLayer<float, HidVec, OutVec, LinearLayout, Sigmoid<float>, true>;

// Flat parameter layout:
// [L1W, L1B, L2W, L2B, L3W, L3B, L4W, L4B]
static const int Addr1W = 0;
static const int Addr1B = Addr1W + HIDDEN_DIM * IN_DIM;
static const int Addr2W = Addr1B + HIDDEN_DIM;
static const int Addr2B = Addr2W + HIDDEN_DIM * HIDDEN_DIM;
static const int Addr3W = Addr2B + HIDDEN_DIM;
static const int Addr3B = Addr3W + HIDDEN_DIM * HIDDEN_DIM;
static const int Addr4W = Addr3B + HIDDEN_DIM;
static const int Addr4B = Addr4W + OUT_DIM * HIDDEN_DIM;
static const int TotalParamCount = Addr4B + OUT_DIM;

[Differentiable]
OutVec runMLP(
    InVec input,
    Storage w1, Storage b1,
    Storage w2, Storage b2,
    Storage w3, Storage b3,
    Storage w4, Storage b4)
{
    LeakyReLU<float> leaky = LeakyReLU<float>(LEAKY_ALPHA);
    Layer1 layer1 = Layer1(leaky);
    Layer2 layer2 = Layer2(leaky);
    Layer3 layer3 = Layer3(leaky);
    Layer4 layer4 = Layer4();

    HidVec h1 = layer1.eval<Storage>(input, w1, Optional<Storage>(b1));
    HidVec h2 = layer2.eval<Storage>(h1, w2, Optional<Storage>(b2));
    HidVec h3 = layer3.eval<Storage>(h2, w3, Optional<Storage>(b3));
    return layer4.eval<Storage>(h3, w4, Optional<Storage>(b4));
}

[AutoPyBindCUDA]
[CUDAKernel]
void forward(
    TensorView<float> input,   // (N, 2)
    TensorView<float> params,  // (33795,)
    TensorView<float> output)  // (N, 3)
{
    uint idx = cudaBlockIdx().x * cudaBlockDim().x + cudaThreadIdx().x;
    uint N = input.size(0);
    bool isActive = idx < N;

    InVec x = InVec(float(0));
    if (isActive)
    {
        for (int i = 0; i < IN_DIM; ++i)
            x[i] = input[idx, i];
    }

    Storage storage = Storage(params);
    OutVec y = runMLP(
        x,
        storage.getOffset(Addr1W), storage.getOffset(Addr1B),
        storage.getOffset(Addr2W), storage.getOffset(Addr2B),
        storage.getOffset(Addr3W), storage.getOffset(Addr3B),
        storage.getOffset(Addr4W), storage.getOffset(Addr4B));

    if (isActive)
    {
        for (int i = 0; i < OUT_DIM; ++i)
            output[idx, i] = y[i];
    }
}

[AutoPyBindCUDA]
[CUDAKernel]
void backward(
    DiffTensorView input,
    DiffTensorView params,
    DiffTensorView output)
{
    uint idx = cudaBlockIdx().x * cudaBlockDim().x + cudaThreadIdx().x;
    uint N = input.size(0);
    bool isActive = idx < N;

    InVec x = InVec(float(0));
    if (isActive)
    {
        for (int i = 0; i < IN_DIM; ++i)
            x[i] = input.primal[idx, i];
    }

    OutVec dL_dOutput = OutVec(float(0));
    if (isActive)
    {
        for (int i = 0; i < OUT_DIM; ++i)
            dL_dOutput[i] = output.diff.diff[idx, i];
    }

    Storage storage = Storage(params.primal);
    Storage gradStorage = Storage(params.diff.diff);

    var w1Pair = DifferentialPtrPair<Storage>(storage.getOffset(Addr1W), gradStorage.getOffset(Addr1W));
    var b1Pair = DifferentialPtrPair<Storage>(storage.getOffset(Addr1B), gradStorage.getOffset(Addr1B));
    var w2Pair = DifferentialPtrPair<Storage>(storage.getOffset(Addr2W), gradStorage.getOffset(Addr2W));
    var b2Pair = DifferentialPtrPair<Storage>(storage.getOffset(Addr2B), gradStorage.getOffset(Addr2B));
    var w3Pair = DifferentialPtrPair<Storage>(storage.getOffset(Addr3W), gradStorage.getOffset(Addr3W));
    var b3Pair = DifferentialPtrPair<Storage>(storage.getOffset(Addr3B), gradStorage.getOffset(Addr3B));
    var w4Pair = DifferentialPtrPair<Storage>(storage.getOffset(Addr4W), gradStorage.getOffset(Addr4W));
    var b4Pair = DifferentialPtrPair<Storage>(storage.getOffset(Addr4B), gradStorage.getOffset(Addr4B));

    InVec dx = InVec(float(0));
    var dpInput = diffPair(x, dx);
    bwd_diff(runMLP)(dpInput, w1Pair, b1Pair, w2Pair, b2Pair, w3Pair, b3Pair, w4Pair, b4Pair, dL_dOutput);

    if (isActive)
    {
        for (int i = 0; i < IN_DIM; ++i)
            input.diff.diff[idx, i] = dpInput.d[i];
    }
}
