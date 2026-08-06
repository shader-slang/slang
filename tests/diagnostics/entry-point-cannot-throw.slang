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

// The `/*CHECK:` block below binds to the last non-blank line above it that is not itself an
// annotation, and matches its carets against that line by column. A comment counts as such a
// line, so inserting one between the declaration and the block retargets the carets at the
// comment instead. Renaming `computeMain` or re-indenting its line moves the target
// horizontally. Both mistakes leave a test that still runs, so re-run it after either.
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
