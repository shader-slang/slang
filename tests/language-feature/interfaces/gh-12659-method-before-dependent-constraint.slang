//TEST:SIMPLE:

// Regression test for https://github.com/shader-slang/slang/issues/12659.
// Computing `Context`'s inheritance for a method signature must not eagerly check
// an unrelated associated-type bound while that computation is incomplete.

interface IContext {}

interface IConsumer<T> where T : IContext {}

interface ILayout
{
    associatedtype Context : IContext;
    IConsumer<Context> getConsumer();
    associatedtype Consumer : IConsumer<Context>;
}
