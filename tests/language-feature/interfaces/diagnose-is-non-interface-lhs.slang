// Test E30300: 'is'/'as' operator on a concrete (non-interface) typed expression.
//
// DISABLED: E30300 (IsOperatorValueMustBeInterfaceType) is unreachable dead code.
// The diagnostic appears in both visitIsTypeExpr and visitAsTypeExpr
// of slang-check-expr.cpp. Both guards require a non-interface type to
// have a subtype relationship, but Slang has no struct inheritance — the only subtype
// relationships are through interface conformance, so isInterfaceType() is always true
// when these guards are reached.

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
