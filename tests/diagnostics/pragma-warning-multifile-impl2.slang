// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
implementing test_pragma_warning;

namespace impl2
{
    interface IConvertibleFrom<From> {}

    // This should still NOT produce warning 30856 (but currently does due to the bug)
    extension<From : __BuiltinFloatingPointType, To : IConvertibleFrom<From>, let N : int>
    vector<To, N> : IConvertibleFrom<vector<From, N>> {}
}