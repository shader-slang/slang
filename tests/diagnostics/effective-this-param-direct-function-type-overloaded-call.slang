//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -no-codegen

// `IOperation.apply` has a `[mutating]` `int` overload and a `[nonmutating]` `float` overload.
// Overload resolution selects the `[mutating]` overload for an `int` argument, after which
// receiver validation diagnoses the immutable `in` receiver. Overload resolution selects the
// `[nonmutating]` overload for a `float` argument, which remains valid. Capturing the `inout` value
// in a lambda likewise makes the selected mutating call's receiver immutable.

interface IOperation
{
    [mutating]
    __associatedfunc functype(int) -> int apply;

    [nonmutating]
    __associatedfunc functype(float) -> int apply;
}

func readFromInput<T : IOperation>(value: T) -> int
{
    return value.apply(1);
//CHECK:              ^ mutating method cannot be called on immutable value
//CHECK:              ^ mutating method 'apply' cannot be called on an immutable value
}

func readThroughNonmutatingOverload<T : IOperation>(value: T) -> int
{
    // Receiver validation must not diagnose the losing mutating candidate after overload
    // resolution selects the `[nonmutating]` overload.
    return value.apply(1.0);
}

func updateInPlace<T : IOperation>(inout value: T) -> int
{
    int directResult = value.apply(1);

    let call = (int argument) => value.apply(argument);
//CHECK:                                    ^ mutating method cannot be called on immutable value
//CHECK:                                    ^ mutating method 'apply' cannot be called on an immutable value
    return directResult + call(1);
}

