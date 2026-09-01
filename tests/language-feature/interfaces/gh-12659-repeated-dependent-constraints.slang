//TEST:INTERPRET(filecheck=CHECK):

// Regression test for https://github.com/shader-slang/slang/issues/12659.
// Resolving the first dependent bound must not eagerly check the second bound while
// `Context`'s inheritance information is incomplete.

interface IContext
{
    int getValue();
}

interface IConsumer<T> where T : IContext
{
    int consume(T context);
}

interface ILayout
{
    associatedtype Context : IContext;
    associatedtype First : IConsumer<Context>;
    associatedtype Second : IConsumer<Context>;

    Context getContext();
    First getFirst();
    Second getSecond();
}

struct ContextImpl : IContext
{
    int value;
    int getValue() { return value; }
}

struct ConsumerImpl : IConsumer<ContextImpl>
{
    int consume(ContextImpl context) { return context.getValue(); }
}

struct LayoutImpl : ILayout
{
    typealias Context = ContextImpl;
    typealias First = ConsumerImpl;
    typealias Second = ConsumerImpl;

    ContextImpl context;

    ContextImpl getContext() { return context; }
    ConsumerImpl getFirst() { return ConsumerImpl(); }
    ConsumerImpl getSecond() { return ConsumerImpl(); }
}

int useLayout<T : ILayout>(T layout)
{
    T.Context context = layout.getContext();
    return layout.getFirst().consume(context) + layout.getSecond().consume(context);
}

void main()
{
    LayoutImpl layout;
    layout.context.value = 21;

    // CHECK: 42
    printf("%d\n", useLayout(layout));
}
