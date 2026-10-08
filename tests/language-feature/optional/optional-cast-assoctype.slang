//TEST:SIMPLE(filecheck=CHECK):-target spirv

// Regression test for issue #7406: the associated-type case.
// Optional<T> should implicitly coerce to Optional<IFoo> when the inner type
// is an associated type constrained to IFoo (i.e. an abstract type witness,
// not a concrete struct). This is the second `///<--- THIS` site in the issue.

interface IFoo
{
    int get();
}

interface IFooUser
{
    // Associated type whose value type is only known to conform to IFoo.
    associatedtype FooConcrete : IFoo;
    Optional<FooConcrete> getFoo();
}

// A concrete user that binds FooConcrete to a real struct.
struct FooImpl : IFoo
{
    int x;
    int get() { return x; }
}

struct FooUser : IFooUser
{
    typealias FooConcrete = FooImpl;
    Optional<FooConcrete> getFoo() { FooImpl f = { 7 }; return f; }
}

// The coercion under test: Optional<This.FooConcrete> -> Optional<IFoo>,
// where FooConcrete is an abstract associated type bounded by IFoo.
__generic<This : IFooUser>
Optional<IFoo> castAssocType(This user)
{
    return user.getFoo(); // implicit coercion: Optional<This.FooConcrete> -> Optional<IFoo>
}

// CHECK-NOT: error[
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
}
