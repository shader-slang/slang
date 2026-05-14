//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry computeMain -capability spvCooperativeMatrixKHR

// Mirror of coopvec-self-differential-dadd.slang for IRCoopMatrixType.
// Verifies that emitDAddOfDiffInstType and emitDZeroOfDiffInstType short-circuit
// to direct IR ops for CoopMat self-differential types.

using namespace linalg;

// Extension generic params need fully-qualified linalg:: names because 'using
// namespace' is not visible to the extension parameter scope at this point.
extension<
    T : __BuiltinFloatingPointType,
    let S : MemoryScope,
    let M : int,
    let N : int,
    let R : linalg::CoopMatMatrixUse
> linalg::CoopMat<T, S, M, N, R> : IDifferentiable
{
    typealias Differential = linalg::CoopMat<T, S, M, N, R>;
};

typealias CoopMatFloat = CoopMat<float, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixAccumulator>;

[Differentiable]
CoopMatFloat addCM(CoopMatFloat a, CoopMatFloat b)
{
    // CoopMat arithmetic is not auto-differentiable, so the primal body is a
    // placeholder. The custom backward below is what the autodiff pass uses;
    // it propagates dResult to both inputs, exercising the emitDAdd path for CoopMat.
    return a;
}

[BackwardDerivativeOf(addCM)]
void addCM_Backward(
    inout DifferentialPair<CoopMatFloat> a,
    inout DifferentialPair<CoopMatFloat> b,
    CoopMatFloat.Differential dResult)
{
    a = diffPair(a.p, dResult);
    b = diffPair(b.p, dResult);
}

[Differentiable]
CoopMatFloat g(CoopMatFloat x)
{
    return addCM(x, x);
}

RWStructuredBuffer<float> output;

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain(uint id: SV_DispatchThreadID)
{
    var x = diffPair(CoopMatFloat(2.0f), CoopMatFloat(0.0f));
    bwd_diff(g)(x, CoopMatFloat(1.0f));

    output[0] = x.d[id];
}

// CHECK: OpCompositeConstruct
// CHECK: OpFAdd
// OpFAdd is the key regression guard: the buggy (pre-fix) code emitted OpUndef in
// place of gradient accumulation instead of a real add, so the absence of OpFAdd
// on the regressed compiler causes this CHECK to fail.
