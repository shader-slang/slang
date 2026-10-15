//TEST:SIMPLE:

// The outer conformance supplies both an associated type and the conformance witness for that
// associated type. Resolving the final `Value` projection must use the concrete `Element : IInner`
// witness before it can inspect the nested `IInner` table.
interface IInner
{
    associatedtype Value;
}

interface IOuter
{
    associatedtype Element : IInner;
}

struct Inner : IInner
{
    typealias Value = int;
}

struct Outer : IOuter
{
    typealias Element = Inner;
}

struct RequiresIntValue<T : IOuter>
    where T.Element.Value == int
{}

RequiresIntValue<Outer> value;
