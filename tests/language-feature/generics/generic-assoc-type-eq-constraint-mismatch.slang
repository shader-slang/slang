//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Negative counterpart for shader-slang/slang#12822.
//
// Reducing an associated-type projection at module scope must not cause a
// genuinely-unsatisfied equality constraint to be accepted. Here
// `BadContainer.Element` resolves to `WrongElement`, which is not
// `ConcreteElement`, so `where T.Element == ConcreteElement` is unsatisfied and
// the specialization must still be rejected with error 38029 at module scope.

interface IHasElement
{
    associatedtype Element;
}

struct ConcreteElement {}
struct WrongElement {}

struct BadContainer : IHasElement
{
    typealias Element = WrongElement;
}

struct RequiresConcreteElement<T : IHasElement> where T.Element == ConcreteElement
{
    int value;
}

RequiresConcreteElement<BadContainer> gBad;
/*CHECK:
^^^^^^^^^^^^^^^^^^^^^^^ type argument doesn't conform to interface
^^^^^^^^^^^^^^^^^^^^^^^ type argument 'WrongElement' does not conform to the required interface 'ConcreteElement'
*/
