//TEST:SIMPLE:

// Ordinary alias resolution can expose a requirement projection that then needs semantic
// resolution before the generic equality constraint can be inspected structurally.
interface IHasElement
{
    associatedtype Element;
}

struct Container : IHasElement
{
    typealias Element = int;
}

typealias ProjectedElement<T : IHasElement> = T.Element;

struct RequiresIntElement<T : IHasElement>
    where ProjectedElement<T> == int
{}

RequiresIntElement<Container> value;
