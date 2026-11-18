//TEST:SIMPLE(filecheck=CHECK): -target llvm-host-ir -emit-cpu-via-llvm -whole-program -O0 -entry main -stage compute

// Companion negative regression for shader-slang/slang#14578: the boundary
// this pins is not the trivial no-initializer-blocks shape (`static int x =
// 5;`, covered by static-init-trivial-dropped.slang) but the fragile one the
// terminator skip exists for — a global whose initializer blocks exist and
// contain only pure instructions (a load of another global and an add). If
// the terminator skip regresses, every initialized global on host targets
// starts surviving the link; that failure must show up here as a module
// constructor appearing where none is owed.

//CHECK-NOT: __slang_global_constructor
//CHECK-NOT: llvm.global_ctors

static int gRef = 3;

static int q = gRef + 1;

[shader("compute")]
[numthreads(1, 1, 1)]
void main() {}
