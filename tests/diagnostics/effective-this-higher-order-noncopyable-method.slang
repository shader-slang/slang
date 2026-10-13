//TEST:SIMPLE: -no-codegen -Wno-38109 -Wno-41016

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
    DifferentialPair<float> firstOrderArgument;
    DifferentialPair<NonCopyableReceiver> firstOrderReceiver;
    DifferentialPair<DifferentialPair<NonCopyableReceiver>> nestedReceiver;
    DifferentialPair<DifferentialPair<float>> argument;

    // Parentheses are transparent to receiver discovery on a higher-order operand.
    let parenthesizedResult = __fwd_diff((NonCopyableReceiver::evaluate))(
        firstOrderReceiver,
        firstOrderArgument);

    let result = __fwd_diff(__fwd_diff(NonCopyableReceiver::evaluate))(
        firstOrderReceiver,
        nestedReceiver,
        argument);

    // Higher-order receiver-mode normalization peels each layer's transparent wrappers before
    // propagating the inner layer's receiver-derived parameters.
    let parenthesizedNestedResult = __fwd_diff((__fwd_diff((NonCopyableReceiver::evaluate))))(
        firstOrderReceiver,
        nestedReceiver,
        argument);

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
