// Shadowing a generic parameter is diagnosed as a warning before Slang 202c
// (shader-slang/slang#13752). The same declarations are errors in Slang 202c;
// see generic-param-shadowing-202c.slang.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -std 2018
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -std 2026

struct MethodGenericShadowsStructGeneric<let N : int>
//CHECK:                                     ^ outer generic parameter 'N' declared here
{
    int f<let N : int>() { return N; }
//CHECK:      ^ generic parameter 'N' shadows a generic parameter from an enclosing scope
//CHECK:      ^ generic parameter 'N' shadows an outer generic parameter with the same name
}

struct LocalShadowsStructGeneric<let N : int>
//CHECK:                             ^ outer generic parameter 'N' declared here
{
    int f() { int N = 5; return N; }
//CHECK:          ^ local variable 'N' shadows a generic parameter from an enclosing scope
//CHECK:          ^ local variable 'N' shadows an outer generic parameter with the same name
}

struct ParamShadowsStructGeneric<let N : int>
//CHECK:                             ^ outer generic parameter 'N' declared here
{
    int f(int N) { return N; }
//CHECK:      ^ parameter 'N' shadows a generic parameter from an enclosing scope
//CHECK:      ^ parameter 'N' shadows an outer generic parameter with the same name
}

int paramShadowsOwnGeneric<let N : int>(int N) { return N; }
//CHECK:                       ^ outer generic parameter 'N' declared here
//CHECK:                                    ^ parameter 'N' shadows a generic parameter from an enclosing scope
//CHECK:                                    ^ parameter 'N' shadows an outer generic parameter with the same name

// When several generic parameters have the name, the nearest enclosing one is reported.
struct ParamShadowsMethodAndStructGeneric<let N : int>
//CHECK:                                      ^ outer generic parameter 'N' declared here
{
    int f<let N : int>(int N) { return N; }
//CHECK:      ^ generic parameter 'N' shadows a generic parameter from an enclosing scope
//CHECK:      ^ generic parameter 'N' shadows an outer generic parameter with the same name
//CHECK:      ^ outer generic parameter 'N' declared here
//CHECK:                   ^ parameter 'N' shadows a generic parameter from an enclosing scope
//CHECK:                   ^ parameter 'N' shadows an outer generic parameter with the same name
}

struct TypeParamShadowsValueParam<let T : int>
//CHECK:                              ^ outer generic parameter 'T' declared here
{
    int f<T>() { return 0; }
//CHECK:  ^ generic parameter 'T' shadows a generic parameter from an enclosing scope
//CHECK:  ^ generic parameter 'T' shadows an outer generic parameter with the same name
}

struct ValueParamShadowsTypeParam<T>
//CHECK:                          ^ outer generic parameter 'T' declared here
{
    int f<let T : int>() { return T; }
//CHECK:      ^ generic parameter 'T' shadows a generic parameter from an enclosing scope
//CHECK:      ^ generic parameter 'T' shadows an outer generic parameter with the same name
}

struct TypeParamShadowsTypeParam<T>
//CHECK:                         ^ outer generic parameter 'T' declared here
{
    int byType<T>() { return 1; }
//CHECK:       ^ generic parameter 'T' shadows a generic parameter from an enclosing scope
//CHECK:       ^ generic parameter 'T' shadows an outer generic parameter with the same name
}

struct LoopVariableShadowsTypeParam<T>
//CHECK:                            ^ outer generic parameter 'T' declared here
{
    int f() { int s = 0; for (int T = 0; T < 2; T++) s += T; return s; }
//CHECK:                          ^ local variable 'T' shadows a generic parameter from an enclosing scope
//CHECK:                          ^ local variable 'T' shadows an outer generic parameter with the same name
}

struct LambdaParamShadowsStructGeneric<let N : int>
//CHECK:                                   ^ outer generic parameter 'N' declared here
{
    int f() { let g = (int N) => N; return g(1); }
//CHECK:                   ^ parameter 'N' shadows a generic parameter from an enclosing scope
//CHECK:                   ^ parameter 'N' shadows an outer generic parameter with the same name
}

struct LocalTypeAliasShadowsTypeParam<T>
//CHECK:                              ^ outer generic parameter 'T' declared here
//CHECK:                              ^ outer generic parameter 'T' declared here
//CHECK:                              ^ outer generic parameter 'T' declared here
{
    int f() { typealias T = int; T x = 1; return x; }
//CHECK:                ^ local type 'T' shadows a generic parameter from an enclosing scope
//CHECK:                ^ local type 'T' shadows an outer generic parameter with the same name
    int unusedAlias() { typealias T = int; return 0; }
//CHECK:                          ^ local type 'T' shadows a generic parameter from an enclosing scope
//CHECK:                          ^ local type 'T' shadows an outer generic parameter with the same name
    int unusedTypedef() { typedef int T; return 0; }
//CHECK:                              ^ local type 'T' shadows a generic parameter from an enclosing scope
//CHECK:                              ^ local type 'T' shadows an outer generic parameter with the same name
}

int localStructShadowsValueParam<let N : int>()
//CHECK:                             ^ outer generic parameter 'N' declared here
{
    struct N { int v; }
//CHECK:   ^ local type 'N' shadows a generic parameter from an enclosing scope
//CHECK:   ^ local type 'N' shadows an outer generic parameter with the same name
    N n;
    n.v = 3;
    return n.v;
}

struct LocalGenericTypesShadowTypeParam<T>
//CHECK:                                ^ outer generic parameter 'T' declared here
//CHECK:                                ^ outer generic parameter 'T' declared here
//CHECK:                                ^ outer generic parameter 'T' declared here
{
    int unusedGenericAlias() { typealias Inner<T> = T; return 0; }
//CHECK:                                       ^ generic parameter 'T' shadows a generic parameter from an enclosing scope
//CHECK:                                       ^ generic parameter 'T' shadows an outer generic parameter with the same name
    int usedGenericAlias() { typealias Inner<T> = T; Inner<int> x = 1; return x; }
//CHECK:                                     ^ generic parameter 'T' shadows a generic parameter from an enclosing scope
//CHECK:                                     ^ generic parameter 'T' shadows an outer generic parameter with the same name
    int genericAliasNamedLikeTypeParam() { typealias T<U> = U; T<int> x = 1; return x; }
//CHECK:                                             ^ local type 'T' shadows a generic parameter from an enclosing scope
//CHECK:                                             ^ local type 'T' shadows an outer generic parameter with the same name
}

// A local declared later in an enclosing block is not yet visible to an earlier nested block.
struct LocalBeforeLaterOuterLocal<let N : int>
//CHECK:                              ^ outer generic parameter 'N' declared here
//CHECK:                              ^ outer generic parameter 'N' declared here
{
    int f()
    {
        int x = 0;
        { int N = 1; x += N; }
//CHECK:      ^ local variable 'N' shadows a generic parameter from an enclosing scope
//CHECK:      ^ local variable 'N' shadows an outer generic parameter with the same name
        int N = 2;
//CHECK:    ^ local variable 'N' shadows a generic parameter from an enclosing scope
//CHECK:    ^ local variable 'N' shadows an outer generic parameter with the same name
        return N + x;
    }
}

// Autodiff clones the generics of a differentiable method; the shadowing is reported once.
struct DifferentiableMethodGeneric<T : __BuiltinFloatingPointType>
//CHECK:                           ^ outer generic parameter 'T' declared here
{
    [Differentiable]
    T f<T : __BuiltinFloatingPointType>(T x) { return x * x; }
//CHECK:^ generic parameter 'T' shadows a generic parameter from an enclosing scope
//CHECK:^ generic parameter 'T' shadows an outer generic parameter with the same name
}

struct Outer<let N : int>
//CHECK:         ^ outer generic parameter 'N' declared here
{
    struct Inner
    {
        int f(int N) { return N; }
//CHECK:          ^ parameter 'N' shadows a generic parameter from an enclosing scope
//CHECK:          ^ parameter 'N' shadows an outer generic parameter with the same name
    }
}

// The following declarations do not shadow a generic parameter, so they get no diagnostic.

// A generic parameter of a declaration that does not enclose the function is not visible there.
struct Unrelated<let K : int> {}
int paramNamedLikeUnrelatedGenericParam(int K) { return K; }

// Unwrapping an optional into a local of the same name stays allowed.
int optionalShadowing(Optional<int> i) { if (let i = i) return i; else return -1; }

// A local that shadows a parameter shadows the parameter, not the generic parameter that the
// parameter already hides, so only the parameter is reported.
struct LocalShadowsParamShadowingGeneric<let N : int>
//CHECK:                                     ^ outer generic parameter 'N' declared here
{
    int f(int N) { { int N = 1; return N; } }
//CHECK:      ^ parameter 'N' shadows a generic parameter from an enclosing scope
//CHECK:      ^ parameter 'N' shadows an outer generic parameter with the same name
}

// A member that reuses the name of a generic parameter of its type is not covered.
struct MemberReusesGenericName<let N : int> { int N; }

// Interface conformance can synthesize a wrapper method whose generic parameters and
// parameters copy the names of the requirement's. Those copies are not written by the
// user, so they do not shadow anything.
interface IGenericMethod { int f<T>(); }
struct GenericMethodWitness<T> : IGenericMethod { int f<U>(int extra = 0) { return 0; } }

interface IValueGenericMethod { int f<let N : int>(); }
struct ValueGenericMethodWitness<let N : int> : IValueGenericMethod { int f<let M : int>(int extra = 0) { return M; } }

interface IParamMethod { int f(int N); }
struct ParamMethodWitness<let N : int> : IParamMethod { int f(int x, int extra = 0) { return x; } }

void useMemberwiseCtor()
{
    MemberReusesGenericName<1> m = { 2 };
}
