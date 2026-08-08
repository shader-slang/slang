// Regression test for shader-slang/slang#12386.
//
// An operation that needs a represented value from an operand whose type is eliminated on this
// target must report a diagnostic against the user's code rather than abort with an internal error.
// An empty struct is eliminated on the CUDA and C++ source targets, so a pointer to a local of that
// type has no value at runtime and a comparison on it has nothing to produce.
//
// Each case is a separate compiler invocation selected by `-D`, because the diagnostic is fatal and
// only the first one in a given compile is reported. The comparison is named in the message, so each
// operator is checked rather than assumed to follow from `==`.

//DIAGNOSTIC_TEST:SIMPLE(diag=EQ):-target cuda -entry computeMain -stage compute -DOP_EQ
//DIAGNOSTIC_TEST:SIMPLE(diag=NE):-target cuda -entry computeMain -stage compute -DOP_NE
//DIAGNOSTIC_TEST:SIMPLE(diag=LT):-target cuda -entry computeMain -stage compute -DOP_LT
//DIAGNOSTIC_TEST:SIMPLE(diag=GT):-target cuda -entry computeMain -stage compute -DOP_GT
//DIAGNOSTIC_TEST:SIMPLE(diag=CPP):-target cpp -entry computeMain -stage compute -DOP_CPP

struct Empty
{
}

RWStructuredBuffer<uint> output;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    Empty value;
    Empty other;
    Ptr<Empty> p = __getAddress(value);
    Ptr<Empty> q = __getAddress(other);

#if defined(OP_EQ)
    output[0] = p == nullptr ? 1 : 0;
//EQ:             ^^ operation is not supported on a value whose type has no representation on this target
//EQ:             ^^ 'cmpEQ' cannot be applied here: one of its operands has a type that this target eliminates during legalization -- an empty struct, or an interface-typed value that is lowered away -- so the operand has no value to act on at runtime. Restructure the code so the operation does not read that value.
#elif defined(OP_NE)
    output[0] = p != q ? 1 : 0;
//NE:             ^^ operation is not supported on a value whose type has no representation on this target
//NE:             ^^ 'cmpNE' cannot be applied here: one of its operands has a type that this target eliminates during legalization -- an empty struct, or an interface-typed value that is lowered away -- so the operand has no value to act on at runtime. Restructure the code so the operation does not read that value.
#elif defined(OP_LT)
    output[0] = p < q ? 1 : 0;
//LT:             ^ operation is not supported on a value whose type has no representation on this target
//LT:             ^ 'cmpLT' cannot be applied here: one of its operands has a type that this target eliminates during legalization -- an empty struct, or an interface-typed value that is lowered away -- so the operand has no value to act on at runtime. Restructure the code so the operation does not read that value.
#elif defined(OP_CPP)
    output[0] = p == nullptr ? 1 : 0;
//CPP:            ^^ operation is not supported on a value whose type has no representation on this target
//CPP:            ^^ 'cmpEQ' cannot be applied here: one of its operands has a type that this target eliminates during legalization -- an empty struct, or an interface-typed value that is lowered away -- so the operand has no value to act on at runtime. Restructure the code so the operation does not read that value.
#elif defined(OP_GT)
    output[0] = p > q ? 1 : 0;
//GT:             ^ operation is not supported on a value whose type has no representation on this target
//GT:             ^ 'cmpGT' cannot be applied here: one of its operands has a type that this target eliminates during legalization -- an empty struct, or an interface-typed value that is lowered away -- so the operand has no value to act on at runtime. Restructure the code so the operation does not read that value.
#else
#error "no operator case selected; pass one of -DOP_EQ/-DOP_NE/-DOP_LT/-DOP_GT/-DOP_CPP"
#endif
}
