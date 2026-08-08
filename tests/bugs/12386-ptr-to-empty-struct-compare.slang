// Regression test for shader-slang/slang#12386.
//
// An empty struct has no runtime representation on the C-family targets, so empty-type
// legalization lowers it to nothing and no storage is declared for a local of that type. Loading
// from or storing to such a variable legalizes away along with it, which is why those cases work.
// A comparison cannot do the same: its operand has no runtime value, but its own result is an
// ordinary `bool`, so there is nothing for it to produce.
//
// That combination used to abort the compiler with `InternalError unexpected: non-simple
// operand(s)!` and write no output at all. It is a limitation of what type legalization can express
// for this program rather than an inconsistency in the IR, so it is now reported against the user's
// code.

//DIAGNOSTIC_TEST:SIMPLE(diag=diag,non-exhaustive):-target cuda -entry computeMain -stage compute

struct Empty
{
}

RWStructuredBuffer<uint> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    Empty value;
    Ptr<Empty> pointer = __getAddress(value);
    output[0] = pointer == nullptr ? 1 : 0;
//diag:                            ^ operation is not supported on a value whose type has no representation on this target
}
