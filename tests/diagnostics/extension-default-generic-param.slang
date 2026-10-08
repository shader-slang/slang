//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry computeMain -stage compute

// An `extension` infers its generic parameters from the target type, so a
// default value for such a parameter is never used. Providing one is rejected.

RWStructuredBuffer<int> outputBuffer;

interface IFoo {}
struct Q : IFoo {}

struct Foo<T>
{
    int get() { return 0; }
}

// Type parameter default.
extension<T = int> Foo<T>
//CHECK:      ^^^ default value on extension generic parameter
//CHECK:      ^^^ an extension's generic parameter 'T' is inferred from its target, so it cannot have a default value
{
    int getExt() { return 1; }
}

// Value (`let`) parameter default.
struct Bar<let N : int>
{
    int get() { return N; }
}

extension<let N : int = 4> Bar<N>
//CHECK:                ^ default value on extension generic parameter
//CHECK:                ^ an extension's generic parameter 'N' is inferred from its target, so it cannot have a default value
{
    int getExt() { return N; }
}

// Constraint and default on the same parameter (a `GenericTypeConstraintDecl` is
// interleaved with the parameter in the member list).
struct Baz<T : IFoo>
{
    int get() { return 0; }
}

extension<T : IFoo = Q> Baz<T>
//CHECK:             ^ default value on extension generic parameter
//CHECK:             ^ an extension's generic parameter 'T' is inferred from its target, so it cannot have a default value
{
    int getExt() { return 1; }
}

// Multiple parameters, only the second has a default: exactly one diagnostic, on
// `U`, none on `T`.
struct Pair<T, U>
{
    int get() { return 0; }
}

extension<T, U = int> Pair<T, U>
//CHECK:         ^^^ default value on extension generic parameter
//CHECK:         ^^^ an extension's generic parameter 'U' is inferred from its target, so it cannot have a default value
{
    int getExt() { return 1; }
}

// Multiple parameters, both with defaults: the per-parameter visitor flags each
// independently, so there must be a diagnostic on both `T` and `U`.
struct Two<T, U>
{
    int get() { return 0; }
}

extension<T = int, U = int> Two<T, U>
//CHECK:      ^^^ default value on extension generic parameter
//CHECK:      ^^^ an extension's generic parameter 'T' is inferred from its target, so it cannot have a default value
//CHECK:               ^^^ default value on extension generic parameter
//CHECK:               ^^^ an extension's generic parameter 'U' is inferred from its target, so it cannot have a default value
{
    int getExt() { return 1; }
}

// Ill-formed default: only the new diagnostic should fire; the default is dropped
// so there is no cascade of "undefined identifier" errors against it.
struct IllFormed<T>
{
    int get() { return 0; }
}

extension<T = ThisNameDoesNotExist> IllFormed<T>
//CHECK:      ^^^^^^^^^^^^^^^^^^^^ default value on extension generic parameter
//CHECK:      ^^^^^^^^^^^^^^^^^^^^ an extension's generic parameter 'T' is inferred from its target, so it cannot have a default value
{
    int getExt() { return 1; }
}

// Negative control: an extension with no defaults must not warn.
struct NoDef<T>
{
    int get() { return 0; }
}

extension<T> NoDef<T>
//CHECK-NOT: default value on extension
{
    int getExt() { return 1; }
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = Foo<float>().getExt() + Bar<3>().getExt();
}
