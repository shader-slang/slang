//TEST:INTERPRET(filecheck=CHECK):

// Exercise equality filtering while the same interface also has dependent sibling constraints.
// When computing the bases of `T.Context`, the equality matches on its right-hand side, while the
// unrelated `First` and `Second` constraints must not be checked re-entrantly.

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
    associatedtype Alias;
    __constraint Alias == Context;
    associatedtype First : IConsumer<Context>;
    associatedtype Second : IConsumer<Context>;

    Context getContext();
    Alias getAlias();
    First getConsumer();
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
    typealias Alias = ContextImpl;
    typealias First = ConsumerImpl;
    typealias Second = ConsumerImpl;

    ContextImpl context;

    ContextImpl getContext() { return context; }
    ContextImpl getAlias() { return context; }
    ConsumerImpl getConsumer() { return ConsumerImpl(); }
}

T.Alias asAlias<T : ILayout>(T.Context context)
{
    return context;
}

T.Context asContext<T : ILayout>(T.Alias alias)
{
    return alias;
}

int useLayout<T : ILayout>(T layout)
{
    T.Alias alias = asAlias<T>(layout.getContext());
    T.Context context = asContext<T>(layout.getAlias());
    return alias.getValue() + layout.getConsumer().consume(context);
}

void main()
{
    LayoutImpl layout;
    layout.context.value = 21;

    // CHECK: 42
    printf("%d\n", useLayout(layout));
}
