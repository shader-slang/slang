//TEST:SIMPLE(filecheck=CHECK):

// Regression test for https://github.com/shader-slang/slang/issues/8148.
// A `static const int` requirement on a generic interface, referenced from the
// `where`-clause of a generic interface method, previously crashed during
// hoistable IR placement (`assert failure: parentNonBlock` in
// `mergeCandidateParentsForHoistableInst`). The front end produced two distinct
// `IRGeneric` instances for the same `IEncoder` interface declaration, so the
// operands of the method's `Specialize` instruction had no common ancestor
// generic, violating dominance. The module must now lower to IR without error.

interface IVector<T, int N> {}

interface IEncoder<T>
{
    static const int In;
    MyVector eval<MyVector>(MyVector input) where MyVector : IVector<T, In>;
}

// CHECK: result code = 0
