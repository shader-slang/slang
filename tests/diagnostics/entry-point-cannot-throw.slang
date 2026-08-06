//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv
//
// Regression test for https://github.com/shader-slang/slang/issues/12330
//
// The two directives above cover the two ways a function becomes an entry point, which are
// separate code paths that both have to reject a `throws` clause: the first names it with
// `-entry` (reaching the check via `findAndValidateEntryPoint`), the second relies on the
// `[shader("compute")]` attribute below (reaching it via `Module::_discoverEntryPointsImpl`).
//
// A shader entry point is invoked by the pipeline, not by other Slang code, so it has
// nowhere to propagate an uncaught error to.

RWStructuredBuffer<uint> outputBuffer;

// A non-entry-point function may still declare `throws`; only the entry point may not.
uint g(uint n) throws uint
{
    if (n < 1)
        throw 0x1U;

    return n;
}

// The carets in the block below are matched by column against `computeMain` on the line they
// follow, and that block must stay immediately after it -- even a comment in between stops the
// annotation from binding. Renaming the entry point or re-indenting its line moves the target
// horizontally and silently misaligns them, so re-run this test after any such edit.
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID) throws uint
/*CHECK:
     ^^^^^^^^^^^ entry point cannot have a 'throws' clause
     ^^^^^^^^^^^ entry point 'computeMain' cannot declare a 'throws' clause; a shader entry point has no error return channel, so handle the error inside the entry point with 'do'/'catch' instead
*/
{
    outputBuffer[tid.x] = try g(tid.x);
}
