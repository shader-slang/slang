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
     ^ concrete type passed to an existential reference parameter
     ^ parameter 'x' requires storage of type 'dyn IThing', but argument has concrete type 'MyThing'. To accept concrete storage, use a generic 'T x' with a 'where T : IThing' constraint.
*/
}
