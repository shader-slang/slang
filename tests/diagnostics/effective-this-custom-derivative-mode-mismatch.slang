//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -Wno-38109

// A member custom derivative must preserve the effective parameter-passing mode of the primal's
// `this` parameter. Matching only the receiver value type would accept these declarations even
// though their call ABIs differ.

struct Receiver
{
    float scale;

    [constref]
    [NoDiffThis]
    float evaluate(float value)
    {
        return scale * value;
    }
}

extension Receiver
{
    [__ref]
    [NoDiffThis]
    [ForwardDerivativeOf(evaluate)]
    /*CHECK:
     ^^^^^^^^^^^^^^^^^^^ custom derivative 'this' type mismatch
     ^^^^^^^^^^^^^^^^^^^ custom derivative does not match expected signature on `this`.
    */
    DifferentialPair<float> evaluateForward(DifferentialPair<float> value)
    {
        return diffPair(scale * value.p, scale * value.d);
    }
}
