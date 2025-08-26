implementing nested_pragma_test;

__include "nested-pragma-impl2.slang";

namespace impl1
{
    interface IConvertibleFrom<From> {}

    // This should NOT produce warning 30856 due to outer pragma disable
    extension<From : __BuiltinIntegerType, To : IConvertibleFrom<From>, let N : int>
    vector<To, N> : IConvertibleFrom<vector<From, N>> {}
}