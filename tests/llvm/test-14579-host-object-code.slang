//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,exit-code=0):-target host-object-code -whole-program -entry main -stage compute

// HostObjectCode is unconditionally a CPU-via-LLVM target, so its compute
// kernels must legalize to the same three-pointer CPU ABI as llvm-host-ir
// (isCPUTargetViaLLVM returns true for it with no flag). This pins that a
// zero-parameter kernel compiles cleanly on that path — before the
// legalization fix the emitted module was verifier-invalid.
[shader("compute")]
[numthreads(1, 1, 1)]
void main() {}
