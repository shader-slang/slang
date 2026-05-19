//TEST:INTERPRET(filecheck=CHECK):

// Regression test for generic type aliases whose default type argument is an
// associated type that has its own interface constraint.

interface IData
{
    int getValue();
}

interface IElement
{
    associatedtype DataType : IData;
}

struct Box<T : IData>
{
    T value;

    __init()
    {
        value = T();
    }

    int read()
    {
        return value.getValue();
    }
}

typealias DefaultedBox<T : IElement, U : IData = T.DataType> = Box<U>;

struct UsesDefault<T : IElement>
{
    DefaultedBox<T> value;
}

struct FloatData : IData
{
    int getValue()
    {
        return 31;
    }
}

struct FloatElement : IElement
{
    typealias DataType = FloatData;
}

UsesDefault<FloatElement> gValue;

void main()
{
    DefaultedBox<FloatElement> box;
    printf("%d\n", box.read());
    // CHECK: 31
}
