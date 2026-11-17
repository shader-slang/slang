//TEST:SIMPLE(filecheck=CHECK):

// Regression test for https://github.com/shader-slang/slang/issues/13996.
// An interface that inherits two applications of a generic interface
// (`IDerived : IBase<int>, IBase<float>`) where both applications share a
// non-generic base interface (`IBase<T> : IRoot`) must register exactly one
// witness table for the shared conformance `S : IRoot`. The canonical-path
// classification previously compared bare declaration pointers, so both
// inheritance applications classified as canonical and allocated two nested
// tables for that one conformance, tripping the duplicate-table assertion in
// `registerConformanceInterfaceCheckingState`. The module must now compile
// without an internal error.

interface IRoot {}
interface IBase<T> : IRoot {}
interface IDerived : IBase<int>, IBase<float> {}
struct S : IDerived {}

// The same diamond with the applications listed in the opposite order: the
// canonical path is whichever application is processed first, so both orderings
// must classify their own path as canonical and the other as a stored proof.
interface IDerivedR : IBase<float>, IBase<int> {}
struct SR : IDerivedR {}

int test(S value)
{
    return 1;
}

int testR(SR value)
{
    return 1;
}

// CHECK: result code = 0
