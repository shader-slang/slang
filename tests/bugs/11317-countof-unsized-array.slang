// `countof` requires a statically known element count. An unsized array has
// none, so it must be rejected with a checker diagnostic rather than lowered to
// a `kIROp_CountOf` that no pass folds and no backend emits (which previously
// produced an internal "unexpected IR opcode" error at emit). (shader-slang/slang#13093)

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

RWStructuredBuffer<int> outBuf;
int gArr[];

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    outBuf[0] = countof(gArr);
    //CHECK: invalid countof argument
    //CHECK: argument to countof must be a type pack, tuple, value pack, or fixed-size array
}
