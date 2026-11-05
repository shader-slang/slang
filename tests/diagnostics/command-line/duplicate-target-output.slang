// duplicate-target-output.slang

// Unlike a session description, a complete command line resolves output paths: two
// whole-program outputs for one target are an error.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -o a.spv -o b.spv
//CHECK: multiple output paths have been specified for target 'spirv'

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain() {}
