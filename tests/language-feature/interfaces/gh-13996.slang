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

// The same diamond with the applications listed in the opposite order.
// Canonicity is selected by the witness that inheritance linearization
// returns, not by enumeration order alone: whichever application the
// linearized proof goes through classifies its own path as canonical and
// records the other as a stored proof, so `SR` guards the mirror-image
// owner/proof assignment from `S`.
interface IDerivedR : IBase<float>, IBase<int> {}
struct SR : IDerivedR {}

// The parameter-taking functions mirror the original report: referencing the
// conforming types forces their conformance checking (and the shared-table
// registration the header describes) during this compile.
int test(S value)
{
    return 1;
}

int testR(SR value)
{
    return 1;
}

// CHECK: result code = 0
