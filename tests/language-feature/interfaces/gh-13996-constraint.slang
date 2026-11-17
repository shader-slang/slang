//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

// Regression test for the generic-constraint path-resolution table in
// https://github.com/shader-slang/slang/issues/13996.
// `checkGenericConstraintConformances` builds canonical inheritance paths for
// constraints, reusing the same path classifier. Here the shared inheritance
// declaration has a *generic* base (`IBase<T> : IRoot<T>`), so the path table
// must enumerate the specialized inheritance references and their specialized
// base types: a bare-decl enumeration would see one `IRoot<T>` requirement
// where the two applications provide distinct requirements `IRoot<int>` and
// `IRoot<float>`. Both inherited requirements are then reached through the
// constraint `U : IDerived` with distinct expected values.

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

RWStructuredBuffer<int> output;

[numthreads(1, 1, 1)]
void computeMain()
{
    S s;
    // calc(10) = 11; calc(10.0) = 12.
    output[0] = viaIDerived(s);  // 23
}

// CHECK: 23
