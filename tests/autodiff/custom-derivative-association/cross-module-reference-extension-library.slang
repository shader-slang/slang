//TEST_IGNORE_FILE:

// Functions with reference parameters declared together with `__func_extension` derivatives, for
// `func-extension-reference-library.slang`. Each extension is rejected, so this module fails to
// compile.

module "cross-module-reference-extension-library";

public float referenceForwardExtended(__ref float a, float x) { return a * x * x; }

__func_extension fwd_diff(referenceForwardExtended)(__ref float a, DifferentialPair<float> x) -> DifferentialPair<float>
{
    return diffPair(a * x.p * x.p, 2.0 * a * x.p * x.d);
}

public float groupSharedForwardExtended(groupshared float a, float x) { return a * x * x; }

__func_extension fwd_diff(groupSharedForwardExtended)(groupshared float a, DifferentialPair<float> x) -> DifferentialPair<float>
{
    return diffPair(a * x.p * x.p, 2.0 * a * x.p * x.d);
}

public float referenceBackwardExtended(__ref float a, float x) { return a * x * x; }

__func_extension bwd_diff(referenceBackwardExtended)(__ref float a, inout DifferentialPair<float> x, float dOut) -> void
{
    x = diffPair(x.p, dOut * 2.0 * a * x.p);
}

public float groupSharedBackwardExtended(groupshared float a, float x) { return a * x * x; }

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

public float referenceApplyExtended(__ref float a, float x) { return a * x * x; }

__func_extension __apply(referenceApplyExtended)(__ref float a, float x) -> Tuple<float, ApplyContext>
{
    return makeTuple(a * x * x, ApplyContext(a, x));
}

public float groupSharedApplyExtended(groupshared float a, float x) { return a * x * x; }

__func_extension __apply(groupSharedApplyExtended)(groupshared float a, float x) -> Tuple<float, ApplyContext>
{
    return makeTuple(a * x * x, ApplyContext(a, x));
}
