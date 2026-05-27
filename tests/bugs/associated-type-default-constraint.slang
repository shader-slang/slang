//TEST:INTERPRET(filecheck=CHECK):

// Regression test for default generic type arguments that are associated types
// with their own interface constraints.

interface IData
{
    int getValue();
}

interface IElement
{
    associatedtype DataType : IData;
}

struct FloatData : IData
{
    int getValue()
    {
        return 17;
    }
}

struct FloatElement : IElement
{
    typealias DataType = FloatData;
}

struct ManualData : IData
{
    int getValue()
    {
        return 31;
    }
}

__generic<T : IElement, U : IData = T.DataType>
U getDefaultAssociatedType()
{
    return U();
}

FloatData gFunctionValue = getDefaultAssociatedType<FloatElement>();

interface IBaz
{
    int getValue();
}

interface IBar
{
    associatedtype BazType : IBaz;
}

interface IFoo
{
    associatedtype BarType : IBar;
}

struct ConcreteBaz : IBaz
{
    int getValue()
    {
        return 23;
    }
}

struct ConcreteBar : IBar
{
    typealias BazType = ConcreteBaz;
}

struct ConcreteFoo : IFoo
{
    typealias BarType = ConcreteBar;
}

__generic<T : IFoo, U : IBar = T.BarType, W : IBaz = U.BazType>
W getChainedDefaultAssociatedType()
{
    return W();
}

ConcreteBaz gChainedFunctionValue = getChainedDefaultAssociatedType<ConcreteFoo>();

struct NestedAssociatedDefault<T : IElement>
{
    U load<U : IData = T.DataType>()
    {
        return U();
    }
}

void main()
{
    printf("%d\n", getDefaultAssociatedType<FloatElement>().getValue());
    // CHECK: 17

    printf("%d\n", getChainedDefaultAssociatedType<ConcreteFoo>().getValue());
    // CHECK: 23

    NestedAssociatedDefault<FloatElement> nested;
    printf("%d\n", nested.load().getValue());
    // CHECK: 17

    printf("%d\n", nested.load<ManualData>().getValue());
    // CHECK: 31
}
