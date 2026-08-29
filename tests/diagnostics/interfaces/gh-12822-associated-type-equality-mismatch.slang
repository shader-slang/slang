//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// On-demand resolution must expose the concrete `float` witness to the generic solver without
// turning a genuine mismatch into an unresolved symbolic comparison or accepting it coinductively.
interface IHasElement
{
    associatedtype Element;
}

struct RequiresIntElement<T : IHasElement>
    where T.Element == int
{}

struct FloatContainer : IHasElement
{
    typealias Element = float;
}

RequiresIntElement<FloatContainer> value;
/*CHECK:
^^^^^^^^^^^^^^^^^^ type argument doesn't conform to interface
^^^^^^^^^^^^^^^^^^ type argument 'float' does not conform to the required interface 'int'
*/
