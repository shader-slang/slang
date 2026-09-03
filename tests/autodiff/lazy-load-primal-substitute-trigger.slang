// A `[PrimalSubstituteOf]` attribute is this module's only autodiff surface: there is no
// differentiability header modifier (`[Differentiable]` / `[ForwardDifferentiable]` /
// `[BackwardDifferentiable]`) and no `fwd_diff`/`bwd_diff` expression. Because the primal-substitute
// attributes do not subclass `DifferentiableAttribute`, the header-modifier path in
// `checkDifferentiableCallableCommon` never fires here. The autodiff supplement is therefore loaded
// solely by the `PrimalSubstituteExpr` synthesized while checking the attribute — the
// `as<PrimalSubstituteExpr>` disjunct of the load trigger in `_checkHigherOrderInvokeExpr`.
//
// The `lazyAutodiffModuleLoading` unit test asserts the matching load-count increment; this test
// guards that the same primal-substitute-only shape still checks successfully end to end.
//
//TEST:SIMPLE:

float original(float x)
{
    return x * x;
}

[PrimalSubstituteOf(original)]
float primalSubst(float x)
{
    return 2.0f * x * x;
}
