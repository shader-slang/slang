//TEST:SIMPLE: -no-codegen

// A failed synthesized constructor must not keep an overload viable. `WithOptional` is not a
// C-style aggregate, so `{1, 2}` is checked against its one-parameter synthesized constructor and
// rejected. `Pair` accepts both elements through legacy aggregate initialization, leaving its
// overload as the only applicable candidate.

struct WithOptional
{
    Optional<int> value;
}

struct Pair
{
    int first;
    int second;
}

int select(WithOptional value)
{
    return 1;
}

int select(Pair value)
{
    return 2;
}

int test()
{
    return select({1, 2});
}

struct NestedOptionalPair
{
    Optional<int> first;
    Optional<int> second;
}

struct NestedAndTail
{
    NestedOptionalPair nested;
    int tail;
}

struct FlatPair
{
    int first;
    int second;
}

int selectSameArity(NestedAndTail value)
{
    return 1;
}

int selectSameArity(FlatPair value)
{
    return 2;
}

int testSameArity()
{
    // `NestedAndTail.init` has two parameters, just like this list has two elements, but its first
    // parameter cannot accept an `int`. Legacy aggregate flattening must not rescue that rejected
    // constructor by consuming both integers as the fields of `nested` and defaulting `tail`.
    return selectSameArity({1, 2});
}

public struct HiddenOnly
{
    private int value;
}

struct VisibleOne
{
    int value;
}

int selectVisibility(HiddenOnly value)
{
    return 1;
}

int selectVisibility(VisibleOne value)
{
    return 2;
}

int testVisibility()
{
    // `HiddenOnly` has no members eligible for a public synthesized constructor. A speculative
    // conversion to that type must reject the candidate without emitting a diagnostic or falling
    // back to legacy aggregate initialization, leaving `VisibleOne` as the applicable overload.
    return selectVisibility({1});
}
