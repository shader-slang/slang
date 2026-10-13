//DIAGNOSTIC_TEST:SIMPLE_EX(diag=CHECK): tests/diagnostics/effective-this-higher-order-overload-selection.slang -no-codegen

// Semantic checking records the actual receiver modes in the higher-order expression's type.
// Overload inference and ranking use a temporary view in which receiver-derived parameters are
// neutral, then final call checking uses the selected expression's actual type.

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
    // Overload resolution selects the mutating overload from the second argument through a
    // receiver-neutral candidate view. Final call checking uses the selected expression's actual
    // writable receiver and rejects this r-value.
    __fwd_diff((Receiver::evaluate))(Receiver(), floatArgument);
//CHECK:                                     ^ argument must be l-value
//CHECK:                                     ^ argument passed to parameter '0' must be l-value.

    // Overload resolution selects the nonmutating overload, leaving an ordinary value receiver.
    __fwd_diff((Receiver::evaluate))(Receiver(), vectorArgument);

    // Generic inference substitutes `T` through a receiver-neutral view. Overload resolution uses
    // the argument type to select this overload, while semantic checking keeps the mutating mode in
    // the expression's actual type for final call checking.
    __fwd_diff((Receiver::evaluateGeneric))(Receiver(), GenericFloatArgument<int>());
//CHECK:                                            ^ argument must be l-value
//CHECK:                                            ^ argument passed to parameter '0' must be l-value.

    // The same inference and selection process leaves the selected nonmutating receiver as an
    // ordinary value.
    __fwd_diff((Receiver::evaluateGeneric))(Receiver(), GenericVectorArgument<int>());
}
