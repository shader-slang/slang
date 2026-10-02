//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv

// Regression test for issue #12965: an overload error must explain a generic candidate that
// matched the argument shape but failed an interface-conformance constraint, instead of listing
// only the concrete overloads.

interface IScalarCapability {}
interface IShapedCapability {}

struct Shape<T>
{
    T value;
}

extension<T : IScalarCapability> Shape<T> : IShapedCapability {}

T choose<T : IShapedCapability>(T left, T right)
{
    return left;
}

int choose(int left, int right) { return left; }
float choose(float left, float right) { return left; }

Shape<T> genericChoose<T>(Shape<T> left, Shape<T> right)
{
    // `T` is unconstrained, so `Shape<T> : IShapedCapability` is unavailable and no overload of
    // `choose` applies.
    return choose(left, right);
}

// One check per emitted diagnostic (SIMPLE mode is exhaustive). The generic candidate is listed
// with a note naming the unsatisfied `IShapedCapability` bound (E40022); the concrete overloads
// keep their argument-mismatch notes.
//CHECK: no overload for 'choose' applicable to arguments of type (Shape<T>, Shape<T>)
//CHECK: candidate: func choose<T> -> T
//CHECK: type argument 'Shape<T>' does not conform to the required interface 'IShapedCapability'
//CHECK: candidate: func choose(int, int) -> int
//CHECK: argument 0 does not match: expected 'int', got 'Shape<T>'
//CHECK: candidate: func choose(float, float) -> float
//CHECK: argument 0 does not match: expected 'float', got 'Shape<T>'
