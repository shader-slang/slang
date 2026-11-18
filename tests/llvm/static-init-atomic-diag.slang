//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target llvm-host-ir -emit-cpu-via-llvm -whole-program -entry main -stage compute

// Regression for shader-slang/slang#14578: an atomic in the initializer of a
// module-scope static that no entry point references was silently dropped on
// the LLVM CPU path — the whole-program link never cloned the global, so the
// module compiled with exit 0, no global constructor and no atomic, dropping
// an observable side effect. The link now preserves host-target globals with
// side-effecting initializers, so the initializer reaches validation and the
// invalid atomic destination is diagnosed, matching the referenced-static
// behavior.

static Atomic<int> g;
static int y = g.add(1);
//CHECK:            ^ use of uninitialized global variable
//CHECK:            ^ use of uninitialized global variable 'g'
//CHECK:            ^ invalid atomic destination
//CHECK:            ^ cannot perform atomic operation because destination is neither groupshared nor from a device buffer.

[shader("compute")]
[numthreads(1, 1, 1)]
void main() {}
