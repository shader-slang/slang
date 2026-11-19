//TEST:SIMPLE(filecheck=CHECK): -stage compute -entry computeMain

// Regression test for the generic-constraint path-resolution table in
// https://github.com/shader-slang/slang/issues/13996.
// This shape already compiled before the fix; it guards the
// specialized-enumeration invariant of the path-resolution table.
// A regression in that invariant surfaces here as a compile failure or
// internal error inside `viaIDerived` (a wrong-application witness no
// longer resolves the right `calc` overload); the computed value itself
// is not checked because executing this constraint path at runtime
// currently trips the separate existential-dispatch internal error.
// `checkGenericConstraintConformances` builds canonical inheritance paths for
// constraints, reusing the same path classifier. Here the shared inheritance
// declaration has a *generic* base (`IBase<T> : IRoot<T>`), so the path table
// must enumerate the specialized inheritance references and their specialized
// base types: a bare-decl enumeration would see one `IRoot<T>` requirement
// where the two applications provide distinct requirements `IRoot<int>` and
// `IRoot<float>`. Both inherited requirements are then reached through the
// constraint `U : IDerived`; successful overload resolution validates that
// each application's requirement stayed distinct.

interface IRoot<T>
{
    int calc(T t);
}

interface IBase<T> : IRoot<T>
{
}

interface IDerived : IBase<int>, IBase<float>
{
}

struct S : IDerived
{
    int calc(int t) { return t + 1; }
    int calc(float t) { return int(t) + 2; }
}

int viaIDerived<U : IDerived>(U u)
{
    // calc resolves through IDerived's inherited closure: IBase<int> -> IRoot<int>
    // for the int overload and IBase<float> -> IRoot<float> for the float overload.
    return u.calc(10) + u.calc(10.0);
}

int computeMain()
{
    S s;
    return viaIDerived(s);
}

// CHECK: result code = 0
