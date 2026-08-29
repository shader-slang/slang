//TEST:SIMPLE: -DCASE_ASSOCIATED_GENERIC_TYPE
//TEST:SIMPLE: -DCASE_ASSOCIATED_GENERIC_FUNCTION
//TEST:SIMPLE: -DCASE_ORDINARY_GENERIC_TYPE
//TEST:SIMPLE: -DCASE_ASSOCIATED_CONFORMANCE_ONLY
//TEST:SIMPLE: -DCASE_ASSOCIATED_GENERIC_TYPE_IN_FUNCTION

// These independently selected cases pin down the original failure boundary. Before the fix, both
// equality and conformance constraints on a concrete associated-type projection failed in a
// module-scope generic type application. The equivalent generic-function application, an ordinary
// type equality, and a function-local generic type application succeeded because they reached
// whole-conformance checking on a different schedule. Keep the cases separate so a future failure
// identifies which schedule changed instead of being hidden by an earlier declaration.

interface IHasElement
{
    associatedtype Element;
}

struct ConcreteElement {}

struct Container : IHasElement
{
    typealias Element = ConcreteElement;
}

#if defined(CASE_ASSOCIATED_GENERIC_TYPE)

struct RequiresConcreteElement<T : IHasElement>
    where T.Element == ConcreteElement
{}

RequiresConcreteElement<Container> value;

#elif defined(CASE_ASSOCIATED_GENERIC_FUNCTION)

void requireConcreteElement<T : IHasElement>(T value)
    where T.Element == ConcreteElement
{}

void useConstraint()
{
    Container value;
    requireConcreteElement(value);
}

#elif defined(CASE_ORDINARY_GENERIC_TYPE)

struct RequiresConcreteType<T>
    where T == ConcreteElement
{}

RequiresConcreteType<ConcreteElement> value;

#elif defined(CASE_ASSOCIATED_CONFORMANCE_ONLY)

interface IMarker {}

struct MarkedElement : IMarker {}

struct MarkedContainer : IHasElement
{
    typealias Element = MarkedElement;
}

struct RequiresMarkedElement<T : IHasElement>
    where T.Element : IMarker
{}

RequiresMarkedElement<MarkedContainer> value;

#elif defined(CASE_ASSOCIATED_GENERIC_TYPE_IN_FUNCTION)

struct RequiresConcreteElement<T : IHasElement>
    where T.Element == ConcreteElement
{}

void useGenericType()
{
    RequiresConcreteElement<Container> value;
}

#endif
