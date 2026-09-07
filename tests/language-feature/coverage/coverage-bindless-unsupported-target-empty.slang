// Companion to coverage-bindless-unsupported-target.slang, covering the case
// where the target rejection is easiest to lose.
//
// The coverage instrumentation pass only runs when the IR contains coverage
// marker ops. This entry point has an empty body, so there is no executable
// statement to instrument, no marker is emitted, and that gate stays shut --
// verified: compiling this shader with `-trace-coverage` alone produces no
// coverage metadata at all. A target-support check living inside the pass
// therefore never fires here, and the compile succeeds in silence.
//
// That silence is the dangerous part: the host asked for one shared descriptor
// binding, did not get it, and would only find out from a pipeline-layout
// mismatch at runtime -- exactly the failure the option exists to remove. So
// the rejection has to happen before the marker-driven gate, and this test
// pins it there.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -stage compute -entry computeMain -profile sm_6_0 -trace-coverage -trace-coverage-bindless-index 3

[numthreads(1, 1, 1)]
void computeMain()
{
}

//CHECK: 45116
