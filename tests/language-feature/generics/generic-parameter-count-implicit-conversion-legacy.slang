//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -std 2026

// Confirm that a speculative conversion-cost query does not suppress the compatibility warning
// when overload resolution later materializes the selected conversion.

interface IWidth<let N : int>
{
}

__generic<T, let N : int>
extension vector<T, N> : IWidth<N>
{
}

struct Converted
{
}

extension Converted
{
    __implicit_conversion(100)
    __init<T : IFloat>(T value)
    {
    }
}

extension Converted
{
    __implicit_conversion(100)
    __init<T : IFloat, let N : int>(T value)
        where T : IWidth<N>
    {
    }
}

void acceptConverted(Converted value)
{
}

void test()
{
    acceptConverted(float3(0.0));
//CHECK:                  ^ deprecated generic-parameter-count overload tie-breaker
//CHECK:                  ^ overload resolution selected an otherwise ambiguous candidate by preferring the single candidate with the fewest required generic parameters; this tie-breaker is removed in Slang 202c

    // A compatibility-selected conversion must not enter the implicit-cast cache. This second
    // conversion therefore resolves the tie again and emits its own warning.
    acceptConverted(float3(1.0));
//CHECK:                  ^ deprecated generic-parameter-count overload tie-breaker
//CHECK:                  ^ overload resolution selected an otherwise ambiguous candidate by preferring the single candidate with the fewest required generic parameters; this tie-breaker is removed in Slang 202c
}
