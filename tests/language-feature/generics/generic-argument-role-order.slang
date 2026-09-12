//TEST:INTERPRET(filecheck=CHECK):

interface IHasElement
{
    associatedtype Element;
    int getValue();
}

struct IntValue : IHasElement
{
    typealias Element = int;
    int value;

    __implicit_conversion(100)
    __init(int value)
    {
        this.value = value;
    }

    int getValue()
    {
        return value;
    }
}

// Generic applications serialize ordinary type/value arguments before hidden witnesses, while
// witnesses retain source constraint order. Put the coercion before the conformance so no consumer
// can treat the conformance as the first hidden argument or place witnesses immediately after the
// type arguments alone. The associated-type equality follows the conformance because its
// `T.Element` endpoint depends on that witness during generic IR lowering (see GH-9334).
int addOffset<T, Element, let offset : int>(T value)
    where T(int) implicit
    where T : IHasElement
    where T.Element == Element
{
    return value.getValue() + offset;
}

void main()
{
    printf("%d\n", addOffset<IntValue, int, 7>(5));
    // CHECK: 12
}
