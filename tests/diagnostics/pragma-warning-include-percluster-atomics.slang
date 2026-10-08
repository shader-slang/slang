implementing pragma_percluster_test;

// First included file (mirrors slangpy's `atomics.slang`): a self-contained
// push/disable/pop cluster around code that would otherwise emit E30856.
// The disable must suppress this file's own warning, and the pop must end the
// suppression at this file's boundary.
namespace percluster_atomics
{
    interface IConvertibleFrom<From> {}

#pragma warning(push)
#pragma warning(disable: 30856)
    // Suppressed by this file's own disable above.
    extension<From : __BuiltinIntegerType, To : IConvertibleFrom<From>, let N : int>
    vector<To, N> : IConvertibleFrom<vector<From, N>> {}
#pragma warning(pop)
}
