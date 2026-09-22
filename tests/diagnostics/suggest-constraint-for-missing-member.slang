// Test for issue #13140: when member lookup fails on a value (or the type) of a
// generic type parameter, the compiler suggests a user-declared interface the
// parameter could be constrained to, as a follow-on note to the "member not
// found" error (e.g. `where Vertex : IHasNormal`).
//
// The suggestion is deliberately conservative (best-effort advice): only
// user-declared, non-generic interfaces are offered, and only when the interface
// declares a member usable in the access that failed (a static access `T.m`
// requires a static requirement).
//
// This runs in exhaustive mode: every emitted diagnostic must be annotated, so
// the negative cases below are enforced automatically — a spurious suggestion
// note would be an unannotated diagnostic and fail the test.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv

interface IHasNormal
{
    float3 getNormal();
}

interface IContainer<T>
{
    T getItem();
}

interface IFactory
{
    static int make();
}

interface IHasDoThing
{
    int doThing();
}

interface IBase
{
    int baseThing();
}

interface IDerived : IBase
{
}

interface IAlpha
{
    int poly();
}

interface IBeta
{
    int poly();
}

interface IHasAssoc
{
    associatedtype assocThing;
}

interface IGenericStaticFactory
{
    static int makeGen<U>(U u);
}

interface IHasStaticSm
{
    static int sm();
}

interface IShared
{
    int shared();
}

namespace NS
{
interface IShared
{
    int shared();
}
}
using namespace NS;

// Positive: a value member access on an unconstrained generic parameter suggests
// the user-declared interface that declares the member.
float3 valueMember<Vertex>(Vertex v)
{
    return v.getNormal();
//CHECK:     ^^^^^^^^^ member not found
//CHECK:     ^^^^^^^^^ 'getNormal' is not a member of 'Vertex'.
//CHECK:     ^^^^^^^^^ consider adding a constraint such as 'where Vertex : IHasNormal', since 'getNormal' is declared by that interface
}

// Positive: a static member access on the type parameter itself suggests an
// interface whose matching requirement is static.
int staticMember<T>()
{
    return T.make();
//CHECK:     ^^^^ member not found
//CHECK:     ^^^^ 'make' is not a member of 'typeof(T)'.
//CHECK:     ^^^^ consider adding a constraint such as 'where T : IFactory', since 'make' is declared by that interface
}

// Positive: the matching static requirement is a *generic* method (`static int makeGen<U>(U)`).
// `getDirectMemberDeclsOfName` returns the outer generic decl and `isDeclUsableAsStaticMember`
// unwraps it to find the `static` modifier on the inner function, so the interface is still
// suggested — this pins that unwrap, which a plain static-modifier check would miss.
int genericStaticRequirement<T>()
{
    return T.makeGen(1);
//CHECK:     ^^^^^^^ member not found
//CHECK:     ^^^^^^^ 'makeGen' is not a member of 'typeof(T)'.
//CHECK:     ^^^^^^^ consider adding a constraint such as 'where T : IGenericStaticFactory', since 'makeGen' is declared by that interface
}

// Positive: a *value* access whose only matching requirement is a static-only member. The value
// path applies no member-kind filter, because Slang resolves `v.sm()` to the static member once the
// parameter is constrained, so `where T : IHasStaticSm` genuinely makes the access valid.
int valueStaticRequirement<T>(T v)
{
    return v.sm();
//CHECK:     ^^ member not found
//CHECK:     ^^ 'sm' is not a member of 'T'.
//CHECK:     ^^ consider adding a constraint such as 'where T : IHasStaticSm', since 'sm' is declared by that interface
}

// Positive: only the interface that *directly* declares the member is suggested; a derived
// interface that merely inherits it is not, because the search does not walk the inheritance graph.
int inheritedMemberDirectOnly<T>(T t)
{
    return t.baseThing();
//CHECK:     ^^^^^^^^^ member not found
//CHECK:     ^^^^^^^^^ 'baseThing' is not a member of 'T'.
//CHECK:     ^^^^^^^^^ consider adding a constraint such as 'where T : IBase', since 'baseThing' is declared by that interface
}

// Negative: only a *generic* interface declares the member. A bare
// `where C : IContainer` is not a valid constraint, so no suggestion is offered.
int genericInterfaceNotSuggested<C>(C c)
{
    return c.getItem();
//CHECK:     ^^^^^^^ member not found
//CHECK:     ^^^^^^^ 'getItem' is not a member of 'C'.
}

// Negative: the access is static (`U.doThing()`) but the only interface declaring
// `doThing` declares it as an instance requirement, so constraining `U` to it
// would not make the static access valid — no suggestion is offered.
int staticInstanceMismatch<U>()
{
    return U.doThing();
//CHECK:     ^^^^^^^ member not found
//CHECK:     ^^^^^^^ 'doThing' is not a member of 'typeof(U)'.
}

// Positive: two differently-named interfaces both declare the member → one note each, emitted in
// a deterministic name-sorted order (IAlpha before IBeta).
int twoMatchesSorted<T>(T t)
{
    return t.poly();
//CHECK:     ^^^^ member not found
//CHECK:     ^^^^ 'poly' is not a member of 'T'.
//CHECK:     ^^^^ consider adding a constraint such as 'where T : IAlpha', since 'poly' is declared by that interface
//CHECK:     ^^^^ consider adding a constraint such as 'where T : IBeta', since 'poly' is declared by that interface
}

// Positive: an associated type is reachable through a value in type position (`v.Element x;`), so a
// value access whose only matching requirement is an associated type is still suggested.
void assocTypeThroughValue<T>(T t)
{
    {
        t.assocThing local;
//CHECK:  ^^^^^^^^^^ member not found
//CHECK:  ^^^^^^^^^^ 'assocThing' is not a member of 'T'.
//CHECK:  ^^^^^^^^^^ consider adding a constraint such as 'where T : IHasAssoc', since 'assocThing' is declared by that interface
    }
}

// Negative: two distinct visible interfaces share the unqualified name `IShared` (one at global
// scope, one imported via `using namespace`), so the bare `where T : IShared` would be ambiguous
// and no suggestion is offered.
int ambiguousNameSuppressed<T>(T t)
{
    return t.shared();
//CHECK:     ^^^^^^ member not found
//CHECK:     ^^^^^^ 'shared' is not a member of 'T'.
}

// Negative: no interface declares the failed name, so no suggestion is offered.
float noSuggestion<Vertex>(Vertex v)
{
    return v.noSuchMember();
//CHECK:     ^^^^^^^^^^^^ member not found
//CHECK:     ^^^^^^^^^^^^ 'noSuchMember' is not a member of 'Vertex'.
}

// Negative: the base is a concrete (non-generic) type, so even though 'getNormal'
// matches an interface, no constraint suggestion is offered.
struct Concrete {}
float3 concreteBase(Concrete c)
{
    return c.getNormal();
//CHECK:     ^^^^^^^^^ member not found
//CHECK:     ^^^^^^^^^ 'getNormal' is not a member of 'Concrete'.
}
