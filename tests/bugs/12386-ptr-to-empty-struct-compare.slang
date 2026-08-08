// Regression test for shader-slang/slang#12386.
//
// An operation that needs a represented value from an operand whose type is eliminated on this
// target must report a diagnostic against the user's code rather than abort with an internal error.
// An empty struct is eliminated on the C-family targets, so a pointer to a local of that type has
// no value at runtime and a comparison on it has nothing to produce.

//DIAGNOSTIC_TEST:SIMPLE(diag=diag):-target cuda -entry computeMain -stage compute

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
//diag:                 ^^ operation is not supported on a value whose type has no representation on this target
//diag:                 ^^ 'cmpEQ' cannot be applied here: one of its operands has a type that is eliminated on this target, such as an empty struct, so the operand has no value at runtime. Restructure the code so the operation does not act on such a value.
}
