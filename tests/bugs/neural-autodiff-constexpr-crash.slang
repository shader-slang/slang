// Regression test for https://github.com/shader-slang/slang/issues/10605
//
// After PR #10391 changed constant folding to use SpecializationConstant
// folding, `static const` members in generic structs computed from generic
// params (e.g. division, ternary) get lowered as constexpr IR ops inside
// the generic. When slang.neural types with such members (like
// WaveTangledVector.Uint4AlignedInputSize) are used in differentiable
// functions, the autodiff pass encounters an orphaned constexpr op (null
// parent, null operands) still referenced by live IR, causing a segfault
// in AutoDiffPass::processReferencedFunctions.
//
//TEST:SIMPLE(filecheck=CHECK): -target cuda -experimental-feature

// CHECK: __global__ void backward
// CHECK: s_bwd_run

import slang.neural;

static const int SubgroupSize = 32;
static const int WARPS = 4;

typealias ShMemSize = SharedMemorySize<half, TargetEnum.CUDA, ExecutionMode.Training, SubgroupSize, WARPS>;
typealias ShMemSizeLayer = ShMemSize.OfLayer4<2, 128, 128, 128, 3>;
typealias InVec = WaveTangledVector<half, ShMemSizeLayer, 2, SubgroupSize>;
typealias HidVec = WaveTangledVector<half, ShMemSizeLayer, 128, SubgroupSize>;
typealias OutVec = WaveTangledVector<half, ShMemSizeLayer, 3, SubgroupSize>;
typealias L1 = FFLayer<half, InVec, HidVec, LinearLayout, LeakyReLU<half>, true>;
typealias L2 = FFLayer<half, HidVec, OutVec, LinearLayout, Sigmoid<half>, true>;
typealias Storage = TorchTensorViewAddress<half>;

[Differentiable]
OutVec run(InVec input, Storage params)
{
    LeakyReLU<half> leaky = LeakyReLU<half>(half(0.01));
    L1 l1 = L1(leaky);
    L2 l2 = L2();

    int offset = 0;
    HidVec h1 = l1.eval<Storage>(input, params.getOffset(offset)); offset += L1.ParameterCount;
    return l2.eval<Storage>(h1, params.getOffset(offset));
}

[CUDAKernel]
void backward(TensorView<half> params, TensorView<half> paramsGrad)
{
    InVec x = InVec(half(0));
    OutVec dOut = OutVec(half(0));
    InVec dx = InVec(half(0));
    var dpInput = diffPair(x, dx);
    bwd_diff(run)(dpInput,
        DifferentialPtrPair<Storage>(Storage(params), Storage(paramsGrad)),
        dOut);
}
