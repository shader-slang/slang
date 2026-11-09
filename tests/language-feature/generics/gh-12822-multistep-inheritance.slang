//TEST:SIMPLE:

// Resolving `Container.Element` must cross two inherited-interface entries before reaching the
// associated-type entry. A frontier-based forceful lookup must therefore materialize one entry,
// restart from the original projection, and repeat rather than treating the first missing entry as
// the requested leaf.
interface IBase
{
    associatedtype Element;
}

interface IMiddle : IBase {}

interface IDerived : IMiddle {}

struct Container : IDerived
{
    typealias Element = int;
}

struct RequiresIntElement<T : IDerived>
    where T.Element == int
{}

RequiresIntElement<Container> value;
