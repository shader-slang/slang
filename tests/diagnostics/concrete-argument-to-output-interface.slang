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
     ^ argument requires packing for an existential box reference parameter
     ^ parameter 'x' has type 'dyn IThing' and is passed by reference, but the argument of type 'MyThing' requires packing into a temporary box. To accept the argument without packing, change only the parameter type to a generic type parameter 'T' constrained by 'where T : IThing', keeping the existing parameter passing mode.
*/
}
