//TEST:INTERPRET(filecheck=CHECK):
interface IGen<A>
{
    associatedtype TB;
    TB getVal();
}

struct Foo1<A> : IGen<A>
{
    typealias TB = int;
    int val = 0;
    TB getVal()
    {
       return val; 
    }
}

struct Foo2<A> : IGen<A>
{
    typealias TB = int;
    int val = 0;
    TB getVal()
    {
       return val; 
    }
}

struct Logic<A1, C1 : IGen<A1>, C2 : IGen<A1>>
{
    int val = 0;
}

extension<A, C1, C2> Logic<A, C1, C2>
    where C1 : IGen<A>
    where C2 : IGen<A>
    where C1.TB == C2.TB
{
    [mutating]
    void setVal(int dataIn)
    {
        val = dataIn;
    }
}

void main()
{
    Logic<int, Foo1<int>, Foo2<int>> logic;
    logic.setVal(42);
    int result = logic.val;
    printf("Result: %d\n", result);
    // CHECK: Result: 42
}