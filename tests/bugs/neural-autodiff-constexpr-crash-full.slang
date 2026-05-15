//TEST:SIMPLE(filecheck=CHECK): -target cuda -experimental-feature

// Full regression test for https://github.com/shader-slang/slang/issues/10605

// 4-layer MLP with autodiff backward pass targeting CUDA. Exercises the
// interaction between `static const` globals computed from macro expansions,
// generic struct members with constexpr arithmetic (division, ternary),
// and the autodiff pass. The original crash was a null-operand segfault in
// AutoDiffPass::processReferencedFunctions.

// CHECK-DAG: __global__ void __kernel__backward
// CHECK-DAG: s_apply_runWaveMLP

import slang.neural;

#ifndef TCNN_MLP_IN_DIM
#define TCNN_MLP_IN_DIM 2
#endif

#ifndef WT_WAVE_WARPS
#define WT_WAVE_WARPS 4
#endif

static const int IN_DIM = TCNN_MLP_IN_DIM;
static const int HIDDEN_DIM = 128;
static const int OUT_DIM = 3;
static const int SubgroupSize = 32;
static const int WARPS_PER_BLOCK = WT_WAVE_WARPS;
static const float LEAKY_ALPHA = 0.01f;

typealias Storage = TorchTensorViewAddress<half>;
typealias ShMemSize = SharedMemorySize<half, TargetEnum.CUDA, ExecutionMode.Training, SubgroupSize, WARPS_PER_BLOCK>;
typealias ShMemSizeLayer = ShMemSize.OfLayer4<IN_DIM, HIDDEN_DIM, HIDDEN_DIM, HIDDEN_DIM, OUT_DIM>;
typealias InVec = WaveTangledVector<half, ShMemSizeLayer, IN_DIM, SubgroupSize, WARPS_PER_BLOCK>;
typealias HidVec = WaveTangledVector<half, ShMemSizeLayer, HIDDEN_DIM, SubgroupSize, WARPS_PER_BLOCK>;
typealias OutVec = WaveTangledVector<half, ShMemSizeLayer, OUT_DIM, SubgroupSize, WARPS_PER_BLOCK>;
typealias Layer1 = FFLayer<half, InVec, HidVec, LinearLayout, LeakyReLU<half>, true>;
typealias Layer2 = FFLayer<half, HidVec, HidVec, LinearLayout, LeakyReLU<half>, true>;
typealias Layer3 = FFLayer<half, HidVec, HidVec, LinearLayout, LeakyReLU<half>, true>;
typealias Layer4 = FFLayer<half, HidVec, OutVec, LinearLayout, Sigmoid<half>, true>;

static const int TotalParamCount =
    Layer1.ParameterCount + Layer2.ParameterCount + Layer3.ParameterCount + Layer4.ParameterCount;

[Differentiable]
OutVec runWaveMLP(InVec input, Storage params)
{
    LeakyReLU<half> leaky = LeakyReLU<half>(half(LEAKY_ALPHA));
    Layer1 layer1 = Layer1(leaky);
    Layer2 layer2 = Layer2(leaky);
    Layer3 layer3 = Layer3(leaky);
    Layer4 layer4 = Layer4();

    int offset = 0;
    HidVec h1 = layer1.eval<Storage>(input, params.getOffset(offset)); offset += Layer1.ParameterCount;
    HidVec h2 = layer2.eval<Storage>(h1,    params.getOffset(offset)); offset += Layer2.ParameterCount;
    HidVec h3 = layer3.eval<Storage>(h2,    params.getOffset(offset)); offset += Layer3.ParameterCount;
    return     layer4.eval<Storage>(h3,    params.getOffset(offset));
}

[AutoPyBindCUDA]
[CUDAKernel]
void backward(
    DiffTensorView input,
    TensorView<half> params,
    TensorView<half> paramsGrad,
    DiffTensorView output)
{
    uint idx = cudaBlockIdx().x * cudaBlockDim().x + cudaThreadIdx().x;
    bool isActive = idx < input.size(0);

    InVec x = InVec(half(0));
    if (isActive)
    {
        for (int i = 0; i < IN_DIM; ++i) x[i] = half(input.primal[idx, i]);
    }

    OutVec dL_dOutput = OutVec(half(0));
    if (isActive)
    {
        for (int i = 0; i < OUT_DIM; ++i) dL_dOutput[i] = half(output.diff.diff[idx, i]);
    }

    InVec dx = InVec(half(0));
    var dpInput = diffPair(x, dx);
    bwd_diff(runWaveMLP)(dpInput,
        DifferentialPtrPair<Storage>(Storage(params), Storage(paramsGrad)),
        dL_dOutput);

    if (isActive)
    {
        for (int i = 0; i < IN_DIM; ++i) input.diff.diff[idx, i] = float(dpInput.d[i]);
    }
}
