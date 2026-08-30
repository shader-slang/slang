//TEST:INTERPRET(filecheck=CHECK):

// Regression test for shader-slang/slang#12753.
//
// An unsuffixed integer literal argument used to infer a generic type parameter
// must not commit that parameter to `int` before an interface-conformance
// constraint on the same parameter has determined it. For `acceptBox(box, 3)`,
// `Element` must be inferred as `uint` from `Box : IBox<Element>` (via
// `UIntBox : IBox<uint>`) and the literal `3` coerced to `uint`, rather than the
// call failing to compile with E38029 ("'UIntBox' does not conform to
// 'IBox<int>'"). Before the fix the calls below did not type-check, so `main`
// never ran and none of the markers were printed.

interface IBox<Element>
{
    Element get();
}

struct UIntBox : IBox<uint>
{
    uint value;
    uint get() { return value; }
}

// The literal `element` argument and the conformance constraint both mention `Element`.
void acceptBox<Element, Box : IBox<Element>>(Box box, Element element) {}

// Order-independent: the literal precedes the conforming argument here.
void acceptBoxReversed<Element, Box : IBox<Element>>(Element element, Box box) {}

// Control: no literal argument, `Element` is inferred purely from conformance.
void inspectBox<Element, Box : IBox<Element>>(Box box) {}

void main()
{
    UIntBox box;
    box.value = 0;

    // The fix: literal `3` is inferred as `uint` from the conformance constraint.
    acceptBox(box, 3);
    //CHECK: accepted
    printf("accepted\n");

    // Order-independent form.
    acceptBoxReversed(4, box);
    //CHECK: accepted-rev
    printf("accepted-rev\n");

    // Explicit `uint(5)` already worked before the fix; must keep working.
    acceptBox(box, uint(5));
    //CHECK: accepted-explicit
    printf("accepted-explicit\n");

    // A `uint`-suffixed literal has a concrete (non-provisional) type and is
    // inferred directly, exercising the suffixed-literal path that is not
    // deferred.
    acceptBox(box, 6u);
    //CHECK: accepted-suffixed
    printf("accepted-suffixed\n");

    // No-literal control; must keep working.
    inspectBox(box);
    //CHECK: inspected
    printf("inspected\n");
}
