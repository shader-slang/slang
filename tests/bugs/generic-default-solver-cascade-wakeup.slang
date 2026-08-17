//TEST:INTERPRET(filecheck=CHECK):

// Regression test for the generic solver work list waking dependent defaults
// after a witness constraint discovers an ordinary argument.

interface IData
{
    int getValue();
}

interface IElement
{
    associatedtype DataType : IData;
}

interface IBox<T : IElement>
{
    T getElement();
}

struct GoodData : IData
{
    int getValue()
    {
        return 64;
    }
}

struct GoodElement : IElement
{
    typealias DataType = GoodData;
}

struct GoodBox : IBox<GoodElement>
{
    GoodElement getElement()
    {
        return GoodElement();
    }
}

__generic<T : IElement, U : IData = T.DataType, Box : IBox<T>>
U readBox(Box box)
{
    return U();
}

void main()
{
    GoodBox box;
    printf("%d\n", readBox(box).getValue());
    // CHECK: 64
}
