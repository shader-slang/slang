// Companion to the other #12367 tests, covering a function-typed global that arrives from a
// precompiled `.slang-module` rather than from the file being compiled.
//
// A deserialized declaration carries no source location, and the module-level checks used to
// require one before reporting -- so this shape compiled with exit 0 and emitted the undefined
// name, which is the defect the diagnostic exists to prevent. Rejecting it is now independent of
// whether a position can be named for it: the error has no `file:line` here, because there is none
// to give, but the output is refused rather than written.
//
// Pinned with a `.expected` comparison rather than diagnostic annotations: those are matched by
// column against a source line, which a diagnostic with no position cannot satisfy.

//TEST:COMPILE: tests/diagnostics/func-type-unsupported-imported-module-lib.slang -o tests/diagnostics/func-type-unsupported-imported-module-lib.slang-module
//TEST:SIMPLE: -r tests/diagnostics/func-type-unsupported-imported-module-lib.slang-module -target cpp -entry computeMain -stage compute

import "func-type-unsupported-imported-module-lib";

RWStructuredBuffer<int> outBuf;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    outBuf[0] = applyIt(gModFn, 41);
}
