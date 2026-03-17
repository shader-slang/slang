// Test E30300: 'is'/'as' operator on a concrete (non-interface) typed expression.
//
// DISABLED: E30300 (IsOperatorValueMustBeInterfaceType) is unreachable dead code.
// The error requires tryGetSubtypeWitness(targetType, valueType) to return non-null
// when valueType is a non-interface type. Since Slang has no struct inheritance, the
// only subtype relationships are through interface conformance, and isInterfaceType()
// returns true for those. The guard at slang-check-expr.cpp:5228 can never fire.

//DISABLE_DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv

[anyValueSize(8)]
interface IFoo
{
    int get();
}

struct Impl : IFoo
{
    int val;
    int get() { return val; }
}

struct Other
{
    int x;
}

void test()
{
    Other o;
    bool b = o is Impl;
    //CHECK: ^ 'is'/'as' operator requires an interface-typed expression
}
