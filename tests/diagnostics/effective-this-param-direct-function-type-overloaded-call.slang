//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -no-codegen

// `IOperation.apply` has a `[mutating]` `int` overload and a `[nonmutating]` `float` overload.
// A call through an `in` generic value must reject the former and accept the latter.
// Capturing an `inout` value in a lambda makes that receiver immutable, so the mutating call must
// also be rejected.

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
    // The losing mutating candidate must not diagnose the selected `[nonmutating]` overload.
    return value.apply(1.0);
}

func updateInPlace<T : IOperation>(inout value: T) -> int
{
    int directResult = value.apply(1);

    let call = (int argument) => value.apply(argument);
//CHECK:                                    ^ mutating method cannot be called on immutable value
//CHECK:                                    ^ mutating method 'apply' cannot be called on an immutable value
//CHECK:                         ^^^^^ attempting to assign to a const variable or immutable member; use '[mutating]' attribute on the containing method to allow modification
    return directResult + call(1);
}
