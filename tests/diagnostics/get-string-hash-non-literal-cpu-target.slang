//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target host-callable -entry main -stage compute

// On a CPU-like target neither `checkGetStringHashInsts` nor `performTypeInlining` runs -- both are
// gated on the same `!isCpuLikeTarget(...)` predicate -- so the pass that would fold this argument
// into a literal never runs, and the pass that would reject it never runs either. A non-literal
// operand is therefore the ordinary shape of legal code here rather than a broken invariant, and the
// emitter's own test of the operand is all that stands between it and reading another instruction's
// storage as string data.
//
// The emitter cannot report E41023 -- that diagnostic belongs to the check that was skipped -- so
// this pins the internal error it does report. `tests/bugs/string-inline.slang` disables its `-cpu`
// and `-llvm` directives against the same gap.

RWStructuredBuffer<uint> outputBuffer;
uniform int sel;

[numthreads(1, 1, 1)]
void main()
{
    String s = sel > 0 ? "aaa" : "bbb";
    outputBuffer[0] = getStringHash(s);
//CHECK: E40100
//CHECK:                           ^ unimplemented feature in Slang compiler: unexpected IR opcode during code emit
}
