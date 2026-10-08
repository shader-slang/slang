//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv

interface IFoo
{
}
interface IBar
{
}

struct MyFoo : IFoo
{
}

Optional<IBar> bad(Optional<MyFoo> v)
{
    return v; // should error: MyFoo does not implement IBar
//CHECK:   ^ type mismatch in expression
//CHECK:   ^ expected an expression of type 'Optional<IBar>', got 'Optional<MyFoo>'
}
