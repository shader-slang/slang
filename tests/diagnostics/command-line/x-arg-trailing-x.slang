// x-arg-trailing-x.slang

// `-X<tool>` forwards exactly one argument, so the trailing `-X` here is read as a tool-name
// prefix with an empty name rather than as a terminator for `--fmad=false`.
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):-target ptx -entry computeMain -stage compute -Xnvrtc --fmad=true --fmad=false -X
// CHECK: error 100004: downstream tool name not defined

[numthreads(1, 1, 1)]
void computeMain() {}
