//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Test that reassigning a `var` after opening its existential type is diagnosed.
// Opening an existential creates a type identity (ExtractExistentialType) tied to
// the variable's declaration. If the variable is reassigned, the concrete type may
// change, invalidating the opened type identity.
// See https://github.com/shader-slang/slang/issues/10004.

interface IFoo
{
    associatedtype Bar;
    Bar getBar();
}

struct Impl1 : IFoo
{
    typedef int Bar;
    int getBar() { return 1; }
}

struct Impl2 : IFoo
{
    typedef float Bar;
    float getBar() { return 2.0; }
}

void test()
{
    IFoo f = Impl1();
    f.Bar b = f.getBar();  // Opens existential on f
    f = Impl2();           // Reassign after existential opened — error
/*CHECK:
      ^ cannot reassign variable after opening existential type
*/
/*CHECK:
      ^ cannot reassign 'f' after its existential type has been opened.
*/
}
