// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
implementing test_pragma_warning;

#pragma warning(push)
#pragma warning(disable: 30856)

namespace impl1
{
    interface IConvertibleFrom<From> {}

    // This should NOT produce warning 30856 due to pragma disable
    extension<From : __BuiltinIntegerType, To : IConvertibleFrom<From>, let N : int>
    vector<To, N> : IConvertibleFrom<vector<From, N>> {}
}

#pragma warning(pop)