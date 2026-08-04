implementing pragma_percluster_test;

// Second included file (mirrors slangpy's `tensor.slang`): the same self-contained
// push/disable/pop cluster. This is the file whose directives #8166 spuriously rejected
// with `warning[E15615] cannot insert pragma here`, because its absolute-location range
// sat below the maximum already recorded by the first included file. Its disable must work
// just like the first file's.
namespace percluster_tensor
{
    interface IConvertibleFrom<From> {}

#pragma warning(push)
#pragma warning(disable: 30856)
    // Suppressed by this file's own disable above; must not trigger E15615.
    extension<From : __BuiltinIntegerType, To : IConvertibleFrom<From>, let N : int>
    vector<To, N> : IConvertibleFrom<vector<From, N>> {}
#pragma warning(pop)
}
