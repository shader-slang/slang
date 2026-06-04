implementing pragma_warning_include_empty_pop;

namespace body_ns
{
    interface IConvertibleFrom<From> {}

    extension<From : __BuiltinIntegerType, To : IConvertibleFrom<From>, let N : int>
    vector<To, N> : IConvertibleFrom<vector<From, N>> {}
}
