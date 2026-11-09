//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// An extern conformance is accepted without local witnesses, but a generic application that needs
// to inspect one of those witnesses structurally cannot force an implementation into existence.
interface IHasElement
{
    associatedtype Element;
}

extern struct External : IHasElement;

struct RequiresIntElement<T : IHasElement>
    where T.Element == int
{}

RequiresIntElement<External> value;
/*CHECK:
^^^^^^^^^^^^^^^^^^ type argument doesn't conform to interface
^^^^^^^^^^^^^^^^^^ type argument 'External.Element' does not conform to the required interface 'int'
*/
