//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):


// Similar to const-to-nodiff-function-diagnostic-improvement.slang, but with a CoopVec type
// to reproduce a more realistic scenario.
extension<T : __BuiltinFloatingPointType, let K : int> CoopVec<T, K> : IDifferentiable
{
    typealias Differential = CoopVec<T, K>;
};

[BackwardDerivativeOf(exp)]
void exp_BackwardAutoDiff<T : __BuiltinFloatingPointType, let K : int>(inout DifferentialPair<CoopVec<T, K>> p0, CoopVec<T, K>.Differential dResult)
{
    p0 = diffPair(p0.p, dResult * exp(p0.p));
}

[Differentiable]
CoopVec<T, K> eval<T : __BuiltinFloatingPointType, let K : int>(CoopVec<T, K> x)
{
    // CHECK-NOT: ([[# @LINE+1]]): error 41020
    return exp(x) - CoopVec<T, K>(1.);
}

[Differentiable]
CoopVec<T, K> eval1<T : __BuiltinFloatingPointType, let K : int>(CoopVec<T, K> x)
{
    // test.slang(25): error 41020: derivative cannot be propagated through call to non-backward-differentiable function `CoopVec.$init`, use 'no_diff' to clarify intention.
    // CHECK: ([[# @LINE+1]]): error 41020
    return exp(x) - CoopVec<T, K>(x[0]);
}


RWStructuredBuffer<float> output;

[shader("compute")]
[numthreads(1,1,1)]
void computeMain(uint id : SV_DispatchThreadID)
{
    var x = diffPair(CoopVec<float, 2>(2.0f), CoopVec<float, 2>(1.0f));
    bwd_diff(eval)(x, CoopVec<float, 2>(1.0f));

    output[0] = x.d[0];

    var x1 = diffPair(CoopVec<float, 2>(2.0f), CoopVec<float, 2>(1.0f));
    bwd_diff(eval1)(x1, CoopVec<float, 2>(1.0f));
    output[1] = x1.d[1];
}

