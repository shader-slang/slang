//TEST:SIMPLE(filecheck=CHECK):
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
    //CHECK: ([[# @LINE+1]]): error 30404: 
    where C1.TB == Wrapper<int>
{
    [mutating]
    void setVal(int dataIn)
    {
        val = dataIn;
    }
}
