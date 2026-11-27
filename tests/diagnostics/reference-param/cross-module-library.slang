//TEST_IGNORE_FILE:

// Functions with reference parameters declared together with requests for their derivatives, for
// `library.slang`. Each request is rejected, so this module fails to compile.

module "cross-module-library";

[Differentiable]
public float referenceSynthesized(__ref float a, float x) { return a * x * x; }

[Differentiable]
public float groupSharedSynthesized(groupshared float a, float x) { return a * x * x; }

[ForwardDerivative(referenceForwardNamedDerivative)]
public float referenceForwardNamed(__ref float a, float x) { return a * x * x; }

public DifferentialPair<float> referenceForwardNamedDerivative(float a, DifferentialPair<float> x)
{
    return diffPair(a * x.p * x.p, 2.0 * a * x.p * x.d);
}

[ForwardDerivative(groupSharedForwardNamedDerivative)]
public float groupSharedForwardNamed(groupshared float a, float x) { return a * x * x; }

public DifferentialPair<float> groupSharedForwardNamedDerivative(float a, DifferentialPair<float> x)
{
    return diffPair(a * x.p * x.p, 2.0 * a * x.p * x.d);
}

[BackwardDerivative(referenceBackwardNamedDerivative)]
public float referenceBackwardNamed(__ref float a, float x) { return a * x * x; }

public void referenceBackwardNamedDerivative(float a, inout DifferentialPair<float> x, float dOut)
{
    x = diffPair(x.p, dOut * 2.0 * a * x.p);
}

[BackwardDerivative(groupSharedBackwardNamedDerivative)]
public float groupSharedBackwardNamed(groupshared float a, float x) { return a * x * x; }

public void groupSharedBackwardNamedDerivative(float a, inout DifferentialPair<float> x, float dOut)
{
    x = diffPair(x.p, dOut * 2.0 * a * x.p);
}

public float referenceForwardOfPrimal(__ref float a, float x) { return a * x * x; }

[ForwardDerivativeOf(referenceForwardOfPrimal)]
public DifferentialPair<float> referenceForwardOfDerivative(float a, DifferentialPair<float> x)
{
    return diffPair(a * x.p * x.p, 2.0 * a * x.p * x.d);
}

public float groupSharedForwardOfPrimal(groupshared float a, float x) { return a * x * x; }

[ForwardDerivativeOf(groupSharedForwardOfPrimal)]
public DifferentialPair<float> groupSharedForwardOfDerivative(float a, DifferentialPair<float> x)
{
    return diffPair(a * x.p * x.p, 2.0 * a * x.p * x.d);
}

public float referenceBackwardOfPrimal(__ref float a, float x) { return a * x * x; }

[BackwardDerivativeOf(referenceBackwardOfPrimal)]
public void referenceBackwardOfDerivative(float a, inout DifferentialPair<float> x, float dOut)
{
    x = diffPair(x.p, dOut * 2.0 * a * x.p);
}

public float groupSharedBackwardOfPrimal(groupshared float a, float x) { return a * x * x; }

[BackwardDerivativeOf(groupSharedBackwardOfPrimal)]
public void groupSharedBackwardOfDerivative(float a, inout DifferentialPair<float> x, float dOut)
{
    x = diffPair(x.p, dOut * 2.0 * a * x.p);
}
