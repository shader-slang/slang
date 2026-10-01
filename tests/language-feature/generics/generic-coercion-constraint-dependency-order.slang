//TEST:INTERPRET(filecheck=CHECK):

interface IContext
{
    associatedtype Element;
}

struct MyFloat
{
    float value;

    __implicit_conversion(100)
    __init(int value)
    {
        this.value = float(value);
    }
}

struct Context : IContext
{
    typealias Element = MyFloat;
}

// Lowering the coercion-witness parameter type needs `T.Element`. The witness that opens that
// associated type is deliberately declared by the later `T : IContext` constraint.
int useElement<T>(T.Element value)
    where T.Element(int) implicit
    where T : IContext
{
    return 17;
}

void main()
{
    MyFloat value = MyFloat(1);
    printf("%d\n", useElement<Context>(value));
    // CHECK: 17
}
