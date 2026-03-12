//DIAGNOSTIC_TEST:SIMPLE(diag=diag):
interface IGen<A>
{
    associatedtype TB;
    int getVal();
}

interface Wrapper<A>
{}

struct Foo1<A> : IGen<A>
{
    typealias TB = Wrapper<A>; // `Wrapper<int>` also fails.
    int val = 0;
    int getVal()
    {
       return val;
    }
}

struct Logic<A1, C1 : IGen<A1>>
{
    int val = 0;
}

extension<A, C1> Logic<A, C1>
    where C1 : IGen<A>
    where C1.TB == Wrapper<int>
//diag:            ^^^^^^^ type 'Wrapper<int>' is not a proper type to use in a generic equality constraint
//diag:            ^^^^^^^ type 'Wrapper<int>' is not a proper type to use in a generic equality constraint
{
    [mutating]
    void setVal(int dataIn)
    {
        val = dataIn;
    }
}
