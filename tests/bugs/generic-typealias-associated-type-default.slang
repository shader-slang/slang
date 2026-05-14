//TEST:SIMPLE: -target hlsl

// Regression test for generic type aliases whose default type argument is an
// associated type that has its own interface constraint.

interface IData
{
}

interface IElement
{
    associatedtype DataType : IData;
}

struct Box<T : IData>
{
}

typealias DefaultedBox<T : IElement, U : IData = T.DataType> = Box<U>;

struct UsesDefault<T : IElement>
{
    DefaultedBox<T> value;
}

struct FloatData : IData
{
}

struct FloatElement : IElement
{
    typealias DataType = FloatData;
}

UsesDefault<FloatElement> gValue;
