//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// Companion to generic-literal-conformance-inference.slang (shader-slang/slang#12753).
// A *suffixed* integer literal has a concrete type the author chose, so it must NOT
// participate in the unsuffixed-literal deferral: `acceptBox(box, 3L)` keeps `Element`
// pinned to the literal's signed type and correctly fails to conform `UIntBox` to it,
// exactly as a non-literal `int` argument would.

interface IBox<Element> {}
struct UIntBox : IBox<uint> {}
void acceptBox<Element, Box : IBox<Element>>(Box box, Element element) {}

void main()
{
    UIntBox box;
    acceptBox(box, 3L);
    //CHECK: E38029
}
