//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -Wno-38109

// Translating the receiver witness for a second forward derivative must form the nested pair from
// the receiver's value type. Its effective BorrowIn wrapper controls parameter passing only; it is
// not itself the value that conforms to IDifferentiable.

[__NonCopyableType]
struct NonCopyableReceiver : IDifferentiable
{
    typealias Differential = float;
    no_diff float value;

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

    [ForwardDifferentiable]
    float evaluate(float scale)
    {
        return value * scale;
    }
}

struct NoDiffBorrowReceiver
{
    float value;

    [constref]
    [NoDiffThis]
    [ForwardDifferentiable]
    float evaluate(float scale)
    {
        return value * scale;
    }
}

void exerciseHigherOrderAdjustedBorrowIn()
{
    DifferentialPair<NonCopyableReceiver> firstOrderReceiver;
    DifferentialPair<DifferentialPair<NonCopyableReceiver>> nestedReceiver;
    DifferentialPair<DifferentialPair<float>> argument;
    let result = __fwd_diff(__fwd_diff(NonCopyableReceiver::evaluate))(
        firstOrderReceiver,
        nestedReceiver,
        argument);

    deliberateHigherOrderStop;
/*CHECK:
    ^^^^^^^^^^^^^^^^^^^^^^^^^ undefined identifier
    ^^^^^^^^^^^^^^^^^^^^^^^^^ undefined identifier 'deliberateHigherOrderStop'.
*/
}

// A NoDiff receiver has no differential witness, but its mode still needs translation when the
// first forward derivative is differentiated again. Reverse-mode synthesis consumes that
// translated receiver in its rematerialization signature.
void exerciseHigherOrderNoDiffBorrowIn()
{
    NoDiffBorrowReceiver receiver;
    DifferentialPair<DifferentialPair<float>> argument;
    DifferentialPair<float> dOut;
    __bwd_diff(__fwd_diff(NoDiffBorrowReceiver::evaluate))(
        receiver,
        receiver,
        argument,
        dOut);
}
