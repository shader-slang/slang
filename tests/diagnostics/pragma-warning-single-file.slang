//TEST:SIMPLE(filecheck=CHECK):

module test_pragma_warning_single;

namespace test1
{
    interface IConvertibleFrom<From> {}

    // This SHOULD produce warning 30856 because it's not disabled
    extension<From : __BuiltinIntegerType, To : IConvertibleFrom<From>, let N : int>
    vector<To, N> : IConvertibleFrom<vector<From, N>> {}
}

#pragma warning(disable: 30856)

namespace test2
{
    interface IConvertibleFrom<From> {}

    // This should NOT produce warning 30856 due to pragma disable
    extension<From : __BuiltinFloatingPointType, To : IConvertibleFrom<From>, let N : int>
    vector<To, N> : IConvertibleFrom<vector<From, N>> {}
}

// CHECK: warning 30856
// CHECK-NOT: 30856