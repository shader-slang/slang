// Test for issue #13140: when member lookup fails on a value (or the type) of a
// generic type parameter, the compiler suggests an interface the parameter could be
// constrained to, as a follow-on note to the "member not found" error (e.g.
// `where Vertex : IHasNormal`, or "consider constraining 'C' to interface
// 'IContainer'" for a generic interface, whose type arguments cannot be inferred).
//
// The suggestion is best-effort advice: an interface is offered only when it declares a
// member usable in the access that failed (a static access `T.m` requires a static
// requirement) and its unqualified name resolves to that interface where the constraint
// would be written.
//
// This runs in exhaustive mode: every emitted diagnostic must be annotated, so the
// negative cases below are enforced automatically — a spurious suggestion note would be
// an unannotated diagnostic and fail the test. `suggest-constraint/applied-compiles.slang`
// checks that each suggested constraint, once applied, compiles and runs.

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

// Negative: inside an interface method with a default body, `this` has the type of a synthesized
// `This` parameter that cannot be given a `where` clause (the fix is `interface IFooUsesBar : IBar`),
// so `IBar` is not suggested.
interface IFooUsesBar
{
    int foo() { return this.bar(); }
//CHECK:                    ^^^ member not found
//CHECK:                    ^^^ 'bar' is not a member of 'This'.
}

interface IBar
{
    int bar();
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

// Positive: a static access whose matching requirement is a *generic* static method
// (`static int makeGen<U>(U)`) is suggested like a non-generic one.
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

// Positive: only the interface that *directly* declares the member is suggested, not a derived
// interface that inherits it; the base is usually visible wherever the derived one is.
int inheritedMemberDirectOnly<T>(T t)
{
    return t.baseThing();
//CHECK:     ^^^^^^^^^ member not found
//CHECK:     ^^^^^^^^^ 'baseThing' is not a member of 'T'.
//CHECK:     ^^^^^^^^^ consider adding a constraint such as 'where T : IBase', since 'baseThing' is declared by that interface
}

// Positive: only a *generic* interface declares the member. A bare `where C : IContainer`
// would not compile and the type argument cannot be inferred, so the note uses advisory wording.
int genericInterfaceSuggested<C>(C c)
{
    return c.getItem();
//CHECK:     ^^^^^^^ member not found
//CHECK:     ^^^^^^^ 'getItem' is not a member of 'C'.
//CHECK:     ^^^^^^^ consider constraining 'C' to interface 'IContainer', since 'getItem' is declared by that interface
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

// Positive: two differently-named interfaces both declare the member → one note each. (The order
// of the notes is checked by `suggest-constraint/sorted-notes.slang`; this exhaustive matcher does
// not check order.)
int twoMatchesSorted<T>(T t)
{
    return t.poly();
//CHECK:     ^^^^ member not found
//CHECK:     ^^^^ 'poly' is not a member of 'T'.
//CHECK:     ^^^^ consider adding a constraint such as 'where T : IAlpha', since 'poly' is declared by that interface
//CHECK:     ^^^^ consider adding a constraint such as 'where T : IBeta', since 'poly' is declared by that interface
}

// Positive: an associated type is reachable through a value in type position (`t.assocThing x;`),
// so a value access whose only matching requirement is an associated type is still suggested.
void assocTypeThroughValue<T>(T t)
{
    t.assocThing local;
/*CHECK:
      ^^^^^^^^^^ member not found
      ^^^^^^^^^^ 'assocThing' is not a member of 'T'.
      ^^^^^^^^^^ consider adding a constraint such as 'where T : IHasAssoc', since 'assocThing' is declared by that interface
*/
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

// Negative: `IHasNormal` declares `getNormal`, but inside this function that name means the first
// generic parameter, so `where T : IHasNormal` would not help and no suggestion is offered.
float3 shadowedByGenericParam<IHasNormal, T>(T value)
{
    return value.getNormal();
//CHECK:         ^^^^^^^^^ member not found
//CHECK:         ^^^^^^^^^ 'getNormal' is not a member of 'T'.
}

// Positive: the shadowing generic parameter belongs to an inner generic, while `T` belongs to the
// struct. The constraint goes on the struct, where `IHasNormal` names the interface, so the note is
// offered (the name is resolved where the constraint is written, not at the failed access).
struct ShadowInMethod<T>
{
    float3 read<IHasNormal>(T value)
    {
        return value.getNormal();
//CHECK:             ^^^^^^^^^ member not found
//CHECK:             ^^^^^^^^^ 'getNormal' is not a member of 'T'.
//CHECK:             ^^^^^^^^^ consider adding a constraint such as 'where T : IHasNormal', since 'getNormal' is declared by that interface
    }
}

// Negative: `v::doThing()` is a static lookup even though `v` is a value, and `IHasDoThing`
// declares `doThing` as an instance requirement, so constraining `U` to it would not help.
int staticSyntaxOnValue<U>(U v)
{
    return v::doThing();
//CHECK:      ^^^^^^^ member not found
//CHECK:      ^^^^^^^ 'doThing' is not a member of 'U'.
}

// Negative: `lessThan` is declared by the core module's `IComparable`, which is in scope for every
// program; core-module interfaces are not suggested.
bool coreInterfaceSkipped<T>(T a, T b)
{
    return a.lessThan(b);
//CHECK:     ^^^^^^^^ member not found
//CHECK:     ^^^^^^^^ 'lessThan' is not a member of 'T'.
}

// Positive: inside `namespace UsedNS`, which the file also opens with `using namespace UsedNS`, the
// scope walk reaches `UsedNS` twice; its interface is still suggested once.
namespace UsedNS
{
interface IUsed
{
    int used();
}
int reachedTwice<T>(T t)
{
    return t.used();
//CHECK:     ^^^^ member not found
//CHECK:     ^^^^ 'used' is not a member of 'T'.
//CHECK:     ^^^^ consider adding a constraint such as 'where T : IUsed', since 'used' is declared by that interface
}
}
using namespace UsedNS;

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
