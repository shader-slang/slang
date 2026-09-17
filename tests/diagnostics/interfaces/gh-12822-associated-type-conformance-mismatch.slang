//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// On-demand resolution must expose the concrete associated type before testing its conformance.
// This is the conformance-direction counterpart to the associated-type equality mismatch test.
interface IMarker
{}

interface IHasElement
{
    associatedtype Element;
}

struct RequiresMarkedElement<T : IHasElement>
    where T.Element : IMarker
{}

struct UnmarkedElement
{}

struct Container : IHasElement
{
    typealias Element = UnmarkedElement;
}

RequiresMarkedElement<Container> value;
/*CHECK:
^^^^^^^^^^^^^^^^^^^^^ type argument doesn't conform to interface
^^^^^^^^^^^^^^^^^^^^^ type argument 'UnmarkedElement' does not conform to the required interface 'IMarker'
*/
