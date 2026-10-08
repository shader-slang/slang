implementing pragma_pushpop_test;

namespace pushpop_body
{
    interface IConvertibleFrom<From> {}

    // This would emit warning 30856, but the root module's `#pragma warning(disable: 30856)`
    // must suppress it (and must keep suppressing it despite the later footer's push/pop).
    extension<From : __BuiltinIntegerType, To : IConvertibleFrom<From>, let N : int>
    vector<To, N> : IConvertibleFrom<vector<From, N>> {}
}
