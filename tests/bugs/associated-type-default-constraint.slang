//TEST:SIMPLE: -target hlsl

// Regression test for default generic type arguments that are associated types
// with their own interface constraints.

interface IData
{
}

interface IElement
{
    associatedtype DataType : IData;
}

struct FloatData : IData
{
}

struct FloatElement : IElement
{
    typealias DataType = FloatData;
}

__generic<T : IElement, U : IData = T.DataType>
U getDefaultAssociatedType()
{
    return U();
}

FloatData gFunctionValue = getDefaultAssociatedType<FloatElement>();

interface IBaz
{
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
