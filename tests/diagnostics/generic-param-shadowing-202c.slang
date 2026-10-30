// Shadowing a generic parameter is an error in Slang 202c (shader-slang/slang#13752).
// Earlier versions report a warning; see generic-param-shadowing.slang.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -std 202c

struct MethodGenericShadowsStructGeneric<let N : int>
//CHECK:                                     ^ outer generic parameter 'N' declared here
{
    int f<let N : int>() { return N; }
//CHECK:      ^ generic parameter 'N' shadows a generic parameter from an enclosing scope, which is not allowed in Slang 202c and later
//CHECK:      ^ generic parameter 'N' shadows an outer generic parameter with the same name
}

struct LocalShadowsStructGeneric<let N : int>
//CHECK:                             ^ outer generic parameter 'N' declared here
{
    int f() { int N = 5; return N; }
//CHECK:          ^ local variable 'N' shadows a generic parameter from an enclosing scope, which is not allowed in Slang 202c and later
//CHECK:          ^ local variable 'N' shadows an outer generic parameter with the same name
}

struct ParamShadowsStructGeneric<let N : int>
//CHECK:                             ^ outer generic parameter 'N' declared here
{
    int f(int N) { return N; }
//CHECK:      ^ parameter 'N' shadows a generic parameter from an enclosing scope, which is not allowed in Slang 202c and later
//CHECK:      ^ parameter 'N' shadows an outer generic parameter with the same name
}

int paramShadowsOwnGeneric<let N : int>(int N) { return N; }
//CHECK:                       ^ outer generic parameter 'N' declared here
//CHECK:                                    ^ parameter 'N' shadows a generic parameter from an enclosing scope, which is not allowed in Slang 202c and later
//CHECK:                                    ^ parameter 'N' shadows an outer generic parameter with the same name

// When several generic parameters have the name, the nearest enclosing one is reported.
struct ParamShadowsMethodAndStructGeneric<let N : int>
//CHECK:                                      ^ outer generic parameter 'N' declared here
{
    int f<let N : int>(int N) { return N; }
//CHECK:      ^ generic parameter 'N' shadows a generic parameter from an enclosing scope, which is not allowed in Slang 202c and later
//CHECK:      ^ generic parameter 'N' shadows an outer generic parameter with the same name
//CHECK:      ^ outer generic parameter 'N' declared here
//CHECK:                   ^ parameter 'N' shadows a generic parameter from an enclosing scope, which is not allowed in Slang 202c and later
//CHECK:                   ^ parameter 'N' shadows an outer generic parameter with the same name
}

struct TypeParamShadowsValueParam<let T : int>
//CHECK:                              ^ outer generic parameter 'T' declared here
{
    int f<T>() { return 0; }
//CHECK:  ^ generic parameter 'T' shadows a generic parameter from an enclosing scope, which is not allowed in Slang 202c and later
//CHECK:  ^ generic parameter 'T' shadows an outer generic parameter with the same name
}

struct ValueParamShadowsTypeParam<T>
//CHECK:                          ^ outer generic parameter 'T' declared here
{
    int f<let T : int>() { return T; }
//CHECK:      ^ generic parameter 'T' shadows a generic parameter from an enclosing scope, which is not allowed in Slang 202c and later
//CHECK:      ^ generic parameter 'T' shadows an outer generic parameter with the same name
}

struct TypeParamShadowsTypeParam<T>
//CHECK:                         ^ outer generic parameter 'T' declared here
{
    int byType<T>() { return 1; }
//CHECK:       ^ generic parameter 'T' shadows a generic parameter from an enclosing scope, which is not allowed in Slang 202c and later
//CHECK:       ^ generic parameter 'T' shadows an outer generic parameter with the same name
}

struct LoopVariableShadowsTypeParam<T>
//CHECK:                            ^ outer generic parameter 'T' declared here
{
    int f() { int s = 0; for (int T = 0; T < 2; T++) s += T; return s; }
//CHECK:                          ^ local variable 'T' shadows a generic parameter from an enclosing scope, which is not allowed in Slang 202c and later
//CHECK:                          ^ local variable 'T' shadows an outer generic parameter with the same name
}

struct LambdaParamShadowsStructGeneric<let N : int>
//CHECK:                                   ^ outer generic parameter 'N' declared here
{
    int f() { let g = (int N) => N; return g(1); }
//CHECK:                   ^ parameter 'N' shadows a generic parameter from an enclosing scope, which is not allowed in Slang 202c and later
//CHECK:                   ^ parameter 'N' shadows an outer generic parameter with the same name
}

struct LocalTypeAliasShadowsTypeParam<T>
//CHECK:                              ^ outer generic parameter 'T' declared here
//CHECK:                              ^ outer generic parameter 'T' declared here
//CHECK:                              ^ outer generic parameter 'T' declared here
{
    int f() { typealias T = int; T x = 1; return x; }
//CHECK:                ^ local type 'T' shadows a generic parameter from an enclosing scope, which is not allowed in Slang 202c and later
//CHECK:                ^ local type 'T' shadows an outer generic parameter with the same name
    int unusedAlias() { typealias T = int; return 0; }
//CHECK:                          ^ local type 'T' shadows a generic parameter from an enclosing scope, which is not allowed in Slang 202c and later
//CHECK:                          ^ local type 'T' shadows an outer generic parameter with the same name
    int unusedTypedef() { typedef int T; return 0; }
//CHECK:                              ^ local type 'T' shadows a generic parameter from an enclosing scope, which is not allowed in Slang 202c and later
//CHECK:                              ^ local type 'T' shadows an outer generic parameter with the same name
}

int localStructShadowsValueParam<let N : int>()
//CHECK:                             ^ outer generic parameter 'N' declared here
{
    struct N { int v; }
//CHECK:   ^ local type 'N' shadows a generic parameter from an enclosing scope, which is not allowed in Slang 202c and later
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
//CHECK:                                       ^ generic parameter 'T' shadows a generic parameter from an enclosing scope, which is not allowed in Slang 202c and later
//CHECK:                                       ^ generic parameter 'T' shadows an outer generic parameter with the same name
    int usedGenericAlias() { typealias Inner<T> = T; Inner<int> x = 1; return x; }
//CHECK:                                     ^ generic parameter 'T' shadows a generic parameter from an enclosing scope, which is not allowed in Slang 202c and later
//CHECK:                                     ^ generic parameter 'T' shadows an outer generic parameter with the same name
    int genericAliasNamedLikeTypeParam() { typealias T<U> = U; T<int> x = 1; return x; }
//CHECK:                                             ^ local type 'T' shadows a generic parameter from an enclosing scope, which is not allowed in Slang 202c and later
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
//CHECK:      ^ local variable 'N' shadows a generic parameter from an enclosing scope, which is not allowed in Slang 202c and later
//CHECK:      ^ local variable 'N' shadows an outer generic parameter with the same name
        int N = 2;
//CHECK:    ^ local variable 'N' shadows a generic parameter from an enclosing scope, which is not allowed in Slang 202c and later
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
//CHECK:^ generic parameter 'T' shadows a generic parameter from an enclosing scope, which is not allowed in Slang 202c and later
//CHECK:^ generic parameter 'T' shadows an outer generic parameter with the same name
}

struct Outer<let N : int>
//CHECK:         ^ outer generic parameter 'N' declared here
{
    struct Inner
    {
        int f(int N) { return N; }
//CHECK:          ^ parameter 'N' shadows a generic parameter from an enclosing scope, which is not allowed in Slang 202c and later
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
//CHECK:      ^ parameter 'N' shadows a generic parameter from an enclosing scope, which is not allowed in Slang 202c and later
//CHECK:      ^ parameter 'N' shadows an outer generic parameter with the same name
}

// A member of a type that reuses the name of a generic parameter of the type, or of an
// enclosing generic, shadows that parameter.
struct MemberReusesGenericName<let N : int>
//CHECK:                           ^ outer generic parameter 'N' declared here
{
    int N;
//CHECK:^ member 'N' shadows a generic parameter from an enclosing scope, which is not allowed in Slang 202c and later
//CHECK:^ member 'N' shadows an outer generic parameter with the same name
}

struct StaticMemberShadowsGeneric<let N : int>
//CHECK:                              ^ outer generic parameter 'N' declared here
{
    static const int N = 1;
//CHECK:             ^ member 'N' shadows a generic parameter from an enclosing scope, which is not allowed in Slang 202c and later
//CHECK:             ^ member 'N' shadows an outer generic parameter with the same name
}

struct NestedTypeShadowsGeneric<T>
//CHECK:                        ^ outer generic parameter 'T' declared here
{
    struct T { int x; };
//CHECK:   ^ member 'T' shadows a generic parameter from an enclosing scope, which is not allowed in Slang 202c and later
//CHECK:   ^ member 'T' shadows an outer generic parameter with the same name
}

struct TypeAliasShadowsGeneric<T>
//CHECK:                       ^ outer generic parameter 'T' declared here
{
    typealias T = int;
//CHECK:      ^ member 'T' shadows a generic parameter from an enclosing scope, which is not allowed in Slang 202c and later
//CHECK:      ^ member 'T' shadows an outer generic parameter with the same name
}

struct MethodShadowsGeneric<let N : int>
//CHECK:                        ^ outer generic parameter 'N' declared here
{
    int N() { return 1; }
//CHECK:^ member 'N' shadows a generic parameter from an enclosing scope, which is not allowed in Slang 202c and later
//CHECK:^ member 'N' shadows an outer generic parameter with the same name
}

struct PropertyShadowsGeneric<let N : int>
//CHECK:                          ^ outer generic parameter 'N' declared here
{
    property N : int { get { return 1; } }
//CHECK:     ^ member 'N' shadows a generic parameter from an enclosing scope, which is not allowed in Slang 202c and later
//CHECK:     ^ member 'N' shadows an outer generic parameter with the same name
}

struct EnumCaseShadowsGeneric<T>
//CHECK:                      ^ outer generic parameter 'T' declared here
{
    enum E { T, U };
//CHECK:     ^ member 'T' shadows a generic parameter from an enclosing scope, which is not allowed in Slang 202c and later
//CHECK:     ^ member 'T' shadows an outer generic parameter with the same name
}

interface IAssociatedTypeShadowsGeneric<T>
//CHECK:                                ^ outer generic parameter 'T' declared here
{
    associatedtype T;
//CHECK:           ^ member 'T' shadows a generic parameter from an enclosing scope, which is not allowed in Slang 202c and later
//CHECK:           ^ member 'T' shadows an outer generic parameter with the same name
}

struct ExtendedGeneric<T> { int x; }
extension<T> ExtendedGeneric<T>
//CHECK:  ^ outer generic parameter 'T' declared here
{
    int T() { return 0; }
//CHECK:^ member 'T' shadows a generic parameter from an enclosing scope, which is not allowed in Slang 202c and later
//CHECK:^ member 'T' shadows an outer generic parameter with the same name
}

struct NestedTypeMemberShadowsGeneric<T>
//CHECK:                              ^ outer generic parameter 'T' declared here
{
    struct Inner { int T; };
//CHECK:               ^ member 'T' shadows a generic parameter from an enclosing scope, which is not allowed in Slang 202c and later
//CHECK:               ^ member 'T' shadows an outer generic parameter with the same name
}

void localTypeMemberShadowsGeneric<T>()
//CHECK:                           ^ outer generic parameter 'T' declared here
{
    struct S { int T; };
//CHECK:           ^ member 'T' shadows a generic parameter from an enclosing scope, which is not allowed in Slang 202c and later
//CHECK:           ^ member 'T' shadows an outer generic parameter with the same name
}

// The fields that autodiff synthesizes for the differential type are not reported again.
struct DifferentiableMemberShadowsGeneric<let N : int> : IDifferentiable
//CHECK:                                      ^ outer generic parameter 'N' declared here
{
    float N;
//CHECK:  ^ member 'N' shadows a generic parameter from an enclosing scope, which is not allowed in Slang 202c and later
//CHECK:  ^ member 'N' shadows an outer generic parameter with the same name
}

// The closure type of a lambda has a field for each captured variable that copies the
// variable's name and location, so capturing a generic parameter, or a local that shadows
// one, adds no diagnostic.
struct LambdaCapturesGenericParam
{
    int g<let K : int>(int v) { let l = (int y) => y + K; return l(v); }
}

int lambdaCapturesShadowingLocal<let N : int>()
//CHECK:                             ^ outer generic parameter 'N' declared here
{
    int N = 1;
//CHECK:^ local variable 'N' shadows a generic parameter from an enclosing scope, which is not allowed in Slang 202c and later
//CHECK:^ local variable 'N' shadows an outer generic parameter with the same name
    let l = (int y) => y + N;
    return l(2);
}

// Interface conformance can synthesize a wrapper method whose generic parameters and
// parameters copy the names of the requirement's. Those copies are not written by the
// user, so they do not shadow anything.
interface IGenericMethod { int f<T>(); }
struct GenericMethodWitness<T> : IGenericMethod { int f<U>(int extra = 0) { return 0; } }

interface IValueGenericMethod { int f<let N : int>(); }
struct ValueGenericMethodWitness<let N : int> : IValueGenericMethod { int f<let M : int>(int extra = 0) { return M; } }

interface IParamMethod { int f(int N); }
struct ParamMethodWitness<let N : int> : IParamMethod { int f(int x, int extra = 0) { return x; } }

// The parameters of a synthesized memberwise constructor copy the names of the fields, so
// they are not reported again.
void useMemberwiseCtor()
{
    MemberReusesGenericName<1> m = { 2 };
    DifferentiableMemberShadowsGeneric<1> d = { 1.0 };
}
