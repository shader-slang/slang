//DIAGNOSTIC_TEST:SIMPLE_EX(diag=CHECK): tests/diagnostics/effective-this-higher-order-overload-selection.slang -no-codegen

// A higher-order expression retains its actual receiver modes. Overload inference and ranking use
// a temporary view in which receiver-derived parameters are neutral, then final call checking uses
// the selected expression's actual type.

struct FloatArgument {}
struct VectorArgument {}
struct GenericFloatArgument<T> {}
struct GenericVectorArgument<T> {}

struct Receiver
{
    [mutating]
    [NoDiffThis]
    [ForwardDifferentiable]
    float evaluate(no_diff FloatArgument argument)
    {
        return 0.0;
    }

    [nonmutating]
    [NoDiffThis]
    [ForwardDifferentiable]
    float evaluate(no_diff VectorArgument argument)
    {
        return 0.0;
    }

    [mutating]
    [NoDiffThis]
    [ForwardDifferentiable]
    float evaluateGeneric<T>(no_diff GenericFloatArgument<T> argument)
    {
        return 0.0;
    }

    [nonmutating]
    [NoDiffThis]
    [ForwardDifferentiable]
    float evaluateGeneric<T>(no_diff GenericVectorArgument<T> argument)
    {
        return 0.0;
    }
};

void selectAfterHigherOrderTransformation(
    FloatArgument floatArgument,
    VectorArgument vectorArgument)
{
    // The second argument selects the mutating overload through a receiver-neutral candidate view.
    // Final call checking uses the selected expression's actual writable receiver and rejects this
    // r-value.
    __fwd_diff(Receiver::evaluate)(Receiver(), floatArgument);
//CHECK:                                   ^ argument must be l-value
//CHECK:                                   ^ argument passed to parameter '0' must be l-value.

    // Selecting the nonmutating overload leaves an ordinary value receiver.
    __fwd_diff(Receiver::evaluate)(Receiver(), vectorArgument);

    // Generic inference substitutes `T` through a receiver-neutral view. The expression retains
    // the mutating mode that final call checking uses after the argument type selects this overload.
    __fwd_diff(Receiver::evaluateGeneric)(Receiver(), GenericFloatArgument<int>());
//CHECK:                                          ^ argument must be l-value
//CHECK:                                          ^ argument passed to parameter '0' must be l-value.

    // The same inference path leaves the selected nonmutating receiver as an ordinary value.
    __fwd_diff(Receiver::evaluateGeneric)(Receiver(), GenericVectorArgument<int>());
}
