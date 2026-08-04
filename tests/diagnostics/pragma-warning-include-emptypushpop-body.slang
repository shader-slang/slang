implementing pragma_emptypushpop_test;

namespace emptypushpop_body
{
    interface IConvertibleFrom<From> {}

    // Would emit warning 30856; must stay suppressed by the root module's disable.
    extension<From : __BuiltinIntegerType, To : IConvertibleFrom<From>, let N : int>
    vector<To, N> : IConvertibleFrom<vector<From, N>> {}
}
