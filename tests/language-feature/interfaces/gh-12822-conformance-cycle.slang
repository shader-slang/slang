//TEST:SIMPLE:

// Resolving one conformance entry may recursively request another entry in the same conformance.
// The outer request owns the check, while the recursive request preserves its symbolic projection.
interface IBase
{
    associatedtype Element;
}

struct Box<T : IBase>
    where T.Element == int
{}

interface IFull : IBase
{
    __constraint Element == int;

    Box<This> makeBox();
}

struct Concrete : IFull
{
    typealias Element = int;

    Box<Concrete> makeBox()
    {
        return {};
    }
}
