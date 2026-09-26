// Companion to func-type-unsupported-on-kernel-target.slang, covering #12367 for a
// function-typed global that is written but never read.
//
// This shape has no `load` of the function value anywhere, yet `cpp`, `metal` and `wgsl`
// still emitted the undefined declaration, because a struct field (and a WGSL global) is
// written out by the ordinary type-emission path whether or not anything reads it. A check
// that only inspected uses inside function bodies would report nothing here while the
// uncompilable output was still produced, so this test is what pins the check to the
// declaration rather than to its uses.
//
// `cuda` is deliberately absent: it discards the dead field entirely and emits no
// reference to the function type, so there is nothing to diagnose there. That target's
// silence is correct behaviour, not a missing diagnostic.
//
// Left in exhaustive mode deliberately: `//CHECK-NOT:` is inert in a `diag=` test, so
// exhaustive matching is the only thing asserting that one mistake yields one error. Adding
// `non-exhaustive` to quiet a future cascading diagnostic would silently drop that.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target cpp -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target metal -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target wgsl -entry computeMain -stage compute

int addOne(int x)
{
    return x + 1;
}

int addTwo(int x)
{
    return x + 2;
}

static functype(int) -> int gFn = addOne;
//CHECK:                    ^^^ function-typed value is not supported on this target
//CHECK:                    ^^^ a function-typed value cannot be represented when generating code for the current target; it is supported only where the value can be resolved at compile time, so name the function directly at the call site (or pass it as an argument that names a function directly) instead of storing it in a variable or selecting it at runtime

RWStructuredBuffer<int> outBuf;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    gFn = addTwo;
    outBuf[0] = 41;
}
