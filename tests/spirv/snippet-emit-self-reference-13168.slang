//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry computeMain -stage compute

// #13168: an instruction reference `%name` inside a snippet emits as an index into the instructions
// already emitted before it, so it must name a strictly-earlier instruction. The parser registers a
// result name before parsing that instruction's own operands, so the self-reference `%a = OpNop %a`
// resolves to its own not-yet-emitted index and slips past the parser's undefined-name check (a
// forward reference is still caught there). validateSpvSnippet diagnoses it (E29001) during
// legalization, before emitSpvSnippet would index its emittedInsts array out of bounds.

RWStructuredBuffer<float> outBuf;

//CHECK: E29001
//CHECK: references instruction %a, which is not defined by an earlier instruction
//CHECK-NOT: E99997
__target_intrinsic(spirv, "%a = OpNop %a")
float selfReference(float a);

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    outBuf[0] = selfReference(1.0f);
}
