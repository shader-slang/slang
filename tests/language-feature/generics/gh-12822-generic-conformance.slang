//TEST:SIMPLE:

// The shared `Container<T> : IHasElement` table stores its witness in declaration context. Lookup
// must select that entry first and then apply the `T -> int` or `T -> float` specialization from the
// concrete conformance witness; storing either specialized answer in the shared table is incorrect.

interface IHasElement
{
    associatedtype Element;
}

struct Container<T> : IHasElement
{
    typealias Element = T;
}

struct RequiresElement<T : IHasElement, U>
    where T.Element == U
{}

RequiresElement<Container<int>, int> intValue;
RequiresElement<Container<float>, float> floatValue;
