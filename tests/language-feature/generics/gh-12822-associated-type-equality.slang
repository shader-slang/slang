//TEST:SIMPLE:

// A module-scope generic application used to compare the still-symbolic `Container.Element`
// projection with `ConcreteElement`. Exercise the declaration contexts in which type syntax can
// request the same concrete requirement witness before whole-conformance checking happens to run.

interface IHasElement
{
    associatedtype Element;
}

struct ConcreteElement {}

struct Container : IHasElement
{
    typealias Element = ConcreteElement;
}

struct RequiresConcreteElement<T : IHasElement>
    where T.Element == ConcreteElement
{}

RequiresConcreteElement<Container> globalValue;

struct Aggregate
{
    RequiresConcreteElement<Container> field;
}

typealias ConcreteRequirement = RequiresConcreteElement<Container>;
ConcreteRequirement aliasValue;

RequiresConcreteElement<Container> makeRequirement()
{
    return {};
}

void consumeRequirement(RequiresConcreteElement<Container> value) {}

struct ExtendedContainer {}

extension ExtendedContainer : IHasElement
{
    typealias Element = ConcreteElement;
}

RequiresConcreteElement<ExtendedContainer> extendedValue;
