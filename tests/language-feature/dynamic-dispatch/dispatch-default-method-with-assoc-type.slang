// Mixed inheritance + associated-conformance regression for the
// findWitnessTableEntryInInheritanceClosure helper added in
// shader-slang/slang#11487.
//
// `IDerived` both inherits from `IBase` AND requires an associated type
// `A : IBase`. The conformance checker emits two distinct nested
// `IRWitnessTable` entries on every `(S : IDerived)` parent table:
//   1. inheritance nested table for `(S : IBase)` — concrete type S
//   2. associated-conformance nested table for `(<S::A> : IBase)` —
//      concrete type S::A (here `Helper`)
// Both nested tables contain an entry with the same globally-unique
// requirement key `IBase.getValue`. An unconstrained recursive walk could
// pick either, depending on entry order. The closure walk implementation
// restricts recursion to nested tables whose concrete type matches the
// outermost table's, so only path (1) is traversed and `S::getValue`
// is returned, never `Helper::getValue`.
//
// The default method `defaultDouble` on `IBase` calls `this.getValue()`
// twice; dispatching it through an `IDerived` existential created via
// `createDynamicObject` exercises exactly this lookup shape on each of
// the two conformances.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

interface IBase
{
    float getValue();

    float defaultDouble()
    {
        return getValue() + getValue();
    }
}

interface IDerived : IBase
{
    associatedtype A : IBase;
    float passthroughA(A a);
}

// `Helper` returns 99.0; if the closure walk wrongly descends into the
// associated-conformance nested table, `defaultDouble()` would observe
// 2 * 99.0 = 198.0 instead of the concrete struct's own IBase.getValue.
struct Helper : IBase
{
    float getValue() { return 99.0; }
}

struct Foo : IDerived
{
    typealias A = Helper;
    float getValue() { return 7.0; }
    float passthroughA(Helper a) { return a.getValue(); }
}

struct Bar : IDerived
{
    typealias A = Helper;
    float getValue() { return 13.0; }
    float passthroughA(Helper a) { return a.getValue(); }
}

//TEST_INPUT: type_conformance Foo:IDerived = 0
//TEST_INPUT: type_conformance Bar:IDerived = 1

//TEST_INPUT: cbuffer(data=[0]):name=type_id
uniform int type_id;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    IDerived a = createDynamicObject<IDerived>(type_id, 0);
    IDerived b = createDynamicObject<IDerived>(1 - type_id, 0);

    // type_id=0: a is Foo (getValue=7), b is Bar (getValue=13).
    // defaultDouble = 2 * getValue(); the inherited dispatch goes through
    // the closure walk on `(Foo : IDerived)` / `(Bar : IDerived)`. If the
    // walk mistakenly returned the associated `Helper : IBase` witness,
    // both outputs would be 198.0.
    outputBuffer[0] = a.defaultDouble();   // CHECK: 14.0
    outputBuffer[1] = b.defaultDouble();   // CHECK: 26.0
}
