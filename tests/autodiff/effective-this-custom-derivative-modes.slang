//TEST:SIMPLE: -no-codegen -Wno-38109

// A custom derivative must have the same effective `this` parameter-passing mode as its primal.
// Merely checking these declarations exercises association and the synthesized AD2 aliases; the
// derivative bodies do not need to be invoked here.

struct BorrowReceiver
{
    float scale;

    [constref]
    [NoDiffThis]
    float eval(float x)
    {
        return scale * x * x;
    }
}

extension BorrowReceiver
{
    [constref]
    [NoDiffThis]
    [ForwardDerivativeOf(eval)]
    DifferentialPair<float> evalForward(DifferentialPair<float> x)
    {
        return diffPair(scale * x.p * x.p, scale * 2.0 * x.p * x.d);
    }

    [constref]
    [NoDiffThis]
    [BackwardDerivativeOf(eval)]
    void evalBackward(inout DifferentialPair<float> x, float dOut)
    {
        x = diffPair(x.p, x.d + scale * 2.0 * x.p * dOut);
    }
}

struct RefReceiver
{
    float scale;

    [__ref]
    [NoDiffThis]
    float eval(float x)
    {
        return scale * x * x;
    }
}

extension RefReceiver
{
    [__ref]
    [NoDiffThis]
    [ForwardDerivativeOf(eval)]
    DifferentialPair<float> evalForward(DifferentialPair<float> x)
    {
        return diffPair(scale * x.p * x.p, scale * 2.0 * x.p * x.d);
    }

    [__ref]
    [NoDiffThis]
    [BackwardDerivativeOf(eval)]
    void evalBackward(inout DifferentialPair<float> x, float dOut)
    {
        x = diffPair(x.p, x.d + scale * 2.0 * x.p * dOut);
    }
}

// A non-copyable receiver adjusts its ordinary In default to BorrowIn for primal calls. Legacy
// custom-derivative signatures nevertheless transform that default as In: the forward derivative
// receives a value pair, and the backward derivative receives an inout value pair.
[__NonCopyableType]
struct AdjustedReceiver : IDifferentiable
{
    typealias Differential = float;
    no_diff float scale;

    [Differentiable]
    static float dzero()
    {
        return 0.0;
    }

    [Differentiable]
    static float dadd(float a, float b)
    {
        return a + b;
    }

    [ForwardDerivative(evalForward)]
    [BackwardDerivative(evalBackward)]
    float eval(float x)
    {
        return scale * x;
    }

    static DifferentialPair<float> evalForward(
        DifferentialPair<AdjustedReceiver> receiver,
        DifferentialPair<float> x)
    {
        return diffPair(receiver.p.scale * x.p, receiver.p.scale * x.d);
    }

    static void evalBackward(
        inout DifferentialPair<AdjustedReceiver> receiver,
        inout DifferentialPair<float> x,
        float dOut)
    {}
}
