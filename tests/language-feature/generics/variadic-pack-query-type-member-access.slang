//TEST:INTERPRET(filecheck=CHECK):
//
// This test covers type-side pack queries that continue parsing as full type
// expressions after `__first(...)` / `__last(...)`.
//
// In particular, we want:
// 1. Associated-type lookup to work through `__first(T).Element`.
// 2. Static member type lookup to work through `__first(...)::Nested`.
// 3. Generic application to continue parsing after `__last(...)::Nested<int>`.

interface IHasElement
{
    associatedtype Element;
}

struct IntElementContainer : IHasElement
{
    typealias Element = int;
}

struct FloatElementContainer : IHasElement
{
    typealias Element = float;
}

struct NestedTypesA
{
    struct Nested
    {
        int value;
    }
}

struct NestedTypesB
{
    struct Nested
    {
        float value;
    }
}

struct GenericNestedA
{
    struct Nested<T>
    {
        T value;
    }
}

struct GenericNestedB
{
    struct Nested<T>
    {
        T value;
    }
}

struct FirstAssocHolder<each T> where nonempty(T) where expand each T : IHasElement
{
    // `__first(T)` should inherit `IHasElement`, so `.Element` resolves here.
    __first(T).Element value;
}

// Parsing should continue after the pack query in `::__Nested`.
typealias FirstNestedType = __first(Tuple<NestedTypesA, NestedTypesB>)::Nested;
// Parsing should also continue through a generic application on the queried type.
typealias LastGenericNestedType = __last(Tuple<GenericNestedA, GenericNestedB>)::Nested<int>;

void main()
{
    FirstAssocHolder<IntElementContainer, FloatElementContainer> assoc = {1};
    FirstNestedType nested = {2};
    LastGenericNestedType genericNested = {3};

    int a = assoc.value;
    int b = nested.value;
    int c = genericNested.value;

    printf("%d\n", a);
    // CHECK: 1
    printf("%d\n", b);
    // CHECK: 2
    printf("%d\n", c);
    // CHECK: 3
}
