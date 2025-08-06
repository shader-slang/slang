implementing nested_pragma_test;

namespace impl2
{
    interface IConvertibleFrom<From> {}

    // This should also NOT produce warning 30856 due to outer pragma disable
    extension<From : __BuiltinFloatingPointType, To : IConvertibleFrom<From>, let N : int>
    vector<To, N> : IConvertibleFrom<vector<From, N>> {}
}