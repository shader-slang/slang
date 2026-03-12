//DIAGNOSTIC_TEST:SIMPLE(diag=diag):

interface IThing
{
    [mutating]
    void f();

    void g();
}

struct MyThing: IThing
{
    int a = 0;

    [mutating]
    void f()
    {
        a = 10;
    }

    void g() {}
}

void g(IThing x)
{
    x.g();
}

void f(inout IThing x)
{
    x.f();
}

void h<T:IThing>(inout T x)
{
    x.f();
}

void entry()
{
    MyThing concrete;
    IThing interfaceTyped = MyThing();

    g(interfaceTyped); // No error
    g(concrete); // No error
    f(interfaceTyped); // No error
    h(concrete); // No error
    f(concrete); // ERROR!
/*diag:
     ^ concrete type passed to interface-typed output parameter
     ^ argument passed to parameter 'x' is of concrete type 'MyThing', but interface-typed output parameters require interface-typed arguments. To allow passing a concrete type to this function, you can replace 'IThing x' with a generic 'T x' and a 'where T : IThing' constraint.
*/
}
