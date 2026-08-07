// 3-level interface inheritance with a default method on the deepest base.
// Companion to dispatch-default-method-inherited.slang for shader-slang/
// slang#11487: the single-hop test only exercises one recursion step in
// findWitnessTableEntryInInheritanceClosure; this case forces the walk to
// descend through ITop -> IMid -> IBase to find the default method's
// requirement key on the deepest nested witness table. A regression that
// broke depth >1 would compile fine on the single-hop test and fail here.
//
// `base_default` is defined on the deepest base `IBase` and calls
// `base_value()` on `this`. Dispatching it through an `ITop` existential
// requires two recursion steps to find `base_default`'s entry on the
// nested `IBase` witness table (ITop wt -> IMid wt -> IBase wt).

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

interface IBase
{
    int base_value();

    // Default method on the deepest base, dispatching to sibling `this`
    // methods on every level of the inheritance chain.
    int base_default(int x)
    {
        return x + base_value();
    }
}

interface IMid : IBase
{
    int mid_value();
}

interface ITop : IMid
{
    int top_value();
}

struct A : ITop
{
    int base_value() { return 1; }
    int mid_value() { return 10; }
    int top_value() { return 100; }
}

struct B : ITop
{
    int base_value() { return 2; }
    int mid_value() { return 20; }
    int top_value() { return 200; }
}

//TEST_INPUT: type_conformance A:ITop = 0
//TEST_INPUT: type_conformance B:ITop = 1

//TEST_INPUT: cbuffer(data=[0]):name=type_id
uniform int type_id;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    ITop a = createDynamicObject<ITop>(type_id, 0);
    ITop b = createDynamicObject<ITop>(1 - type_id, 0);

    // type_id=0 -> a is A: base_default(7) = 7 + 1 = 8.
    outputBuffer[0] = a.base_default(7);  // CHECK: 8
    // 1 - type_id = 1 -> b is B: base_default(7) = 7 + 2 = 9.
    outputBuffer[1] = b.base_default(7);  // CHECK: 9
}
