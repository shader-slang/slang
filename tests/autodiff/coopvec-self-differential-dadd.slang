//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry computeMain -capability spvCooperativeVectorNV

extension<T : __BuiltinFloatingPointType, let K : int> CoopVec<T, K> : IDifferentiable
{
    typealias Differential = CoopVec<T, K>;
};

[BackwardDerivativeOf(exp)]
void exp_BackwardAutoDiff<T : __BuiltinFloatingPointType, let K : int>(
    inout DifferentialPair<CoopVec<T, K>> p0,
    CoopVec<T, K>.Differential dResult)
{
    p0 = diffPair(p0.p, dResult * exp(p0.p));
}

[Differentiable]
CoopVec<float, 2> addCV(CoopVec<float, 2> a, CoopVec<float, 2> b)
{
    return a + b;
}

[BackwardDerivativeOf(addCV)]
void addCV_BackwardAutoDiff(
    inout DifferentialPair<CoopVec<float, 2>> a,
    inout DifferentialPair<CoopVec<float, 2>> b,
    CoopVec<float, 2>.Differential dResult)
{
    a = diffPair(a.p, dResult);
    b = diffPair(b.p, dResult);
}

[Differentiable]
CoopVec<float, 2> f(CoopVec<float, 2> x)
{
    return addCV(exp(x), exp(x));
}

RWStructuredBuffer<float> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint id: SV_DispatchThreadID)
{
    var x = diffPair(CoopVec<float, 2>(2.0f), CoopVec<float, 2>(0.0f));
    bwd_diff(f)(x, CoopVec<float, 2>(1.0f));

    output[0] = x.d[0];
}

// CHECK: OpCompositeConstructReplicateEXT
// CHECK: OpFAdd
// Verify the short-circuit fired: no calls to synthesized dadd/dzero helpers.
// CHECK-NOT: OpFunctionCall {{.*}}dadd
// CHECK-NOT: OpFunctionCall {{.*}}dzero
