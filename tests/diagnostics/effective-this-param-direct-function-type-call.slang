//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -no-codegen

// `apply` is an associated function-type requirement. After overload resolution selects that
// requirement from its argument type, semantic checking must reject a call through an ordinary
// `in` value and accept a call through an `inout` value because the requirement is `[mutating]`.

interface IOperation
{
    [mutating]
    __associatedfunc functype(int) -> int apply;
}

func readFromInput<T : IOperation>(value: T) -> int
{
    return value.apply(1);
//CHECK:              ^ mutating method cannot be called on immutable value
//CHECK:              ^ mutating method 'apply' cannot be called on an immutable value
}

func updateInPlace<T : IOperation>(inout value: T) -> int
{
    return value.apply(1);
}
