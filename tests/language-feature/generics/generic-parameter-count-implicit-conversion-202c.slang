//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -std 202c

// Confirm that conversion-cost queries honor Slang 202c overload-resolution rules.

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
/*CHECK:
    ^^^^^^ see declaration of 'Converted.init'
*/
    {
    }
}

extension Converted
{
    __implicit_conversion(100)
    __init<T : IFloat, let N : int>(T value)
/*CHECK:
    ^^^^^^ see declaration of 'Converted.init'
*/
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
//CHECK:                  ^ ambiguous conversion
//CHECK:                  ^ more than one conversion exists from 'vector<float,3>' to 'Converted'
}
