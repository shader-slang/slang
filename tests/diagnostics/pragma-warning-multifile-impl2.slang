// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
implementing test_pragma_warning;

namespace impl2
{
    interface IConvertibleFrom<From> {}

    // impl1's `#pragma warning(disable: 30856)` is wrapped in push/pop and popped before impl1
    // ends, so it does not leak here; this extension therefore correctly emits E30856. (Whether a
    // push/pop-scoped disable should persist across `__include` files is a separate semantic
    // question — see shader-slang/slang#11473.)
    extension<From : __BuiltinFloatingPointType, To : IConvertibleFrom<From>, let N : int>
    vector<To, N> : IConvertibleFrom<vector<From, N>> {}
}
