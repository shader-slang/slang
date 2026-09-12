//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Regression test for shader-slang/slang#12822.
//
// A generic struct constrained by an associated-type projection must specialize
// at module scope for every declaration form (global, field, typealias,
// function parameter/return), not only inside a function body, for both the
// equality projection (`where T.Element == ConcreteElement`) and the conformance
// projection (`where T.Element : IMarker`). Every declaration below is valid and
// must compile without a diagnostic; the default exhaustive `diag` mode fails on
// any diagnostic the compiler emits, so no per-line annotation is needed
// (`//CHECK-NOT:` is not a recognized annotation in this mode). The genuine-
// mismatch-still-rejected case lives in the sibling `...-mismatch.slang`.

interface IMarker
{
    int marker();
}

interface IHasElement
{
    associatedtype Element;
}

struct ConcreteElement : IMarker
{
    int marker() { return 42; }
}

struct Container : IHasElement
{
    typealias Element = ConcreteElement;
}

// A `class` base exercises the other common concrete-aggregate kind.
class ContainerClass : IHasElement
{
    typealias Element = ConcreteElement;
}

// The satisfying conformance may also come from an extension rather than the
// type's own declaration; the projection must reduce through it too, for both a
// struct and an enum subject.
struct ExtendedContainer {}
extension ExtendedContainer : IHasElement
{
    typealias Element = ConcreteElement;
}

enum ExtendedEnum
{
    A,
    B,
}
extension ExtendedEnum : IHasElement
{
    typealias Element = ConcreteElement;
}

// A chained projection: the constraint `T.Element.Element == ConcreteElement`
// requires reducing one projection, then a second on the result.
interface IHasElementChain
{
    associatedtype Element : IHasElementChain;
}

struct ChainLeaf : IHasElementChain
{
    typealias Element = ChainLeaf;
}

struct ChainMid : IHasElementChain
{
    typealias Element = ChainLeaf;
}

struct ChainOuter : IHasElementChain
{
    typealias Element = ChainMid;
}

struct RequiresConcreteElement<T : IHasElement> where T.Element == ConcreteElement
{
    int value;
}

struct RequiresMarkerElement<T : IHasElement> where T.Element : IMarker
{
    int value;
}

struct RequiresChainedLeaf<T : IHasElementChain> where T.Element.Element == ChainLeaf
{
    int value;
}

RequiresConcreteElement<Container> gEq;
RequiresMarkerElement<Container> gConf;
RequiresConcreteElement<ContainerClass> gClassEq;
RequiresConcreteElement<ExtendedContainer> gExtEq;
RequiresMarkerElement<ExtendedContainer> gExtConf;
RequiresConcreteElement<ExtendedEnum> gEnumEq;
RequiresMarkerElement<ExtendedEnum> gEnumConf;
RequiresChainedLeaf<ChainOuter> gChained;

struct Holder
{
    RequiresConcreteElement<Container> eqField;
    RequiresMarkerElement<Container> confField;
}

typealias AliasEq = RequiresConcreteElement<Container>;
typealias AliasConf = RequiresMarkerElement<Container>;

// `ContainerAlias` exercises the projection subject arriving through an alias.
typealias ContainerAlias = Container;
RequiresConcreteElement<ContainerAlias> gAliasedArgEq;
RequiresMarkerElement<ContainerAlias> gAliasedArgConf;

RequiresConcreteElement<Container> makeEq(RequiresConcreteElement<Container> x)
{
    return x;
}

RequiresMarkerElement<Container> makeConf(RequiresMarkerElement<Container> x)
{
    return x;
}

// A function-body control: module- and function-scope specialization must stay
// consistent for both projection kinds.
void useLocals()
{
    RequiresConcreteElement<Container> localEq;
    RequiresMarkerElement<Container> localConf;
}
