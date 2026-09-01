//TEST:INTERPRET(filecheck=CHECK):

// Regression test for https://github.com/shader-slang/slang/issues/12659.
// Computing `Context`'s inheritance for a method signature must not eagerly check
// an unrelated associated-type bound while that computation is incomplete.

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
    Context getContext();
    IConsumer<Context> getConsumer();
    associatedtype Consumer : IConsumer<Context>;
}

// The declarations above reproduce the original re-entrant check. The concrete conformances and
// generic call below also verify that the accepted interface produces usable witnesses for both
// `Context : IContext` and `IConsumer<Context>`.
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
    typealias Consumer = ConsumerImpl;

    ContextImpl context;

    ContextImpl getContext() { return context; }

    IConsumer<ContextImpl> getConsumer()
    {
        ConsumerImpl consumer;
        return consumer;
    }
}

int useLayout<T : ILayout>(T layout)
{
    return layout.getConsumer().consume(layout.getContext());
}

void main()
{
    LayoutImpl layout;
    layout.context.value = 42;

    // CHECK: 42
    printf("%d\n", useLayout(layout));
}
