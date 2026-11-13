//TEST:INTERPRET(filecheck=CHECK):

interface IContext
{
    associatedtype TraceContext;
}

interface ISchema
{
    associatedtype TraceContext;
}

struct TraceContext
{}

struct Context : IContext
{
    typealias TraceContext = ::TraceContext;
}

struct Schema : ISchema
{
    typealias TraceContext = ::TraceContext;
}

struct Tracer<S>
    where S : ISchema
{}

extension<S> Tracer<S>
    where S : ISchema
{
    // The equality requirement needs the later IContext witness before
    // `Actual.TraceContext` can be resolved. Because this method is nested in a
    // generic extension, its solver must also preserve the enclosing value of S.
    int useContext<Actual>()
        where Actual.TraceContext == S.TraceContext
        where Actual : IContext
    {
        return 23;
    }
}

void main()
{
    printf("%d\n", Tracer<Schema>().useContext<Context>());
    // CHECK: 23
}
