//TEST:SIMPLE(filecheck=CHECK): -target llvm-host-ir -emit-cpu-via-llvm -whole-program -O0 -entry main -stage compute

// Companion negative regression for shader-slang/slang#14578: an unreferenced
// module-scope static whose initializer is a pure constant expression has no
// observable effect, so the host-target link must keep dropping it — no
// __slang_global_constructor and no llvm.global_ctors entry should appear. The
// side-effecting variants (static-init-atomic-diag.slang,
// static-init-benign-ctor.slang) are the ones the link preserves.

//CHECK-NOT: __slang_global_constructor
//CHECK-NOT: llvm.global_ctors

static int x = 5;

[shader("compute")]
[numthreads(1, 1, 1)]
void main() {}
