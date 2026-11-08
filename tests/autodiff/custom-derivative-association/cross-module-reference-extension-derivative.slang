//TEST_IGNORE_FILE:

// `__func_extension` derivatives for functions with reference parameters from other modules, for
// `func-extension-reference-derivative-library.slang`. Each extension is rejected, so this module
// fails to compile.

module "cross-module-reference-extension-derivative";

import "cross-module-reference-primal";
__exported import "cross-module-exported-reference-primal";

__func_extension fwd_diff(referenceForwardExtended)(__ref float a, DifferentialPair<float> x) -> DifferentialPair<float>
{
    return diffPair(a * x.p * x.p, 2.0 * a * x.p * x.d);
}

__func_extension bwd_diff(groupSharedBackwardExtended)(groupshared float a, inout DifferentialPair<float> x, float dOut) -> void
{
    x = diffPair(x.p, dOut * 2.0 * a * x.p);
}

public struct ApplyContext
{
    public float a;
    public float x;
    public void operator()(__none_t na, out float dx, float dOut)
    {
        dx = dOut * 2.0 * a * x;
    }
}

__func_extension __apply(referenceApplyExtended)(__ref float a, float x) -> Tuple<float, ApplyContext>
{
    return makeTuple(a * x * x, ApplyContext(a, x));
}

__func_extension __apply(exportedReferenceApplyExtended)(__ref float a, float x) -> Tuple<float, ApplyContext>
{
    return makeTuple(a * x * x, ApplyContext(a, x));
}

__func_extension bwd_diff(exportedGroupSharedBackwardExtended)(groupshared float a, inout DifferentialPair<float> x, float dOut) -> void
{
    x = diffPair(x.p, dOut * 2.0 * a * x.p);
}
