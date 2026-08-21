//TEST:SIMPLE:

// Regression test for https://github.com/shader-slang/slang/issues/12659.
// Resolving the first dependent bound must not eagerly check the second bound while
// `Context`'s inheritance information is incomplete.

interface IContext {}

interface IConsumer<T> where T : IContext {}

interface ILayout
{
    associatedtype Context : IContext;
    associatedtype First : IConsumer<Context>;
    associatedtype Second : IConsumer<Context>;
}
