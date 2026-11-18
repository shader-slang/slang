//TEST:SIMPLE(filecheck=CHECK): -target llvm-host-ir -emit-cpu-via-llvm -whole-program -O0 -entry main -stage compute

// Regression for shader-slang/slang#14578: the link now preserves host-target
// globals whose initializers might have side effects. The module constructor
// must initialize each preserved global before another preserved global's
// initializer reads it — including a global that is preserved only as a
// dependency. `a` and `b` are preserved by their own side-effecting
// initializers; `q` has a pure initializer (a load and an add) and is
// preserved only because `b`'s initializer reads it. The ordered CHECK
// directives below pin the constructor's execution order: initialize `a`,
// then `q` (reading `a`), then `b` (reading `q`). Pinned at -O0 like
// static-init-benign-ctor.slang: at higher optimization levels LLVM may
// soundly fold provably-dead initializers away.

//CHECK-DAG: llvm.global_ctors{{.*}}__slang_global_constructor
//CHECK: define internal void @__slang_global_constructor
//CHECK: call i32 @{{.*}}computeA
//CHECK: store i32 {{.*}}, ptr @{{.*}}1a
//CHECK: load i32, ptr @{{.*}}1a
//CHECK: store i32 {{.*}}, ptr @{{.*}}1q
//CHECK: load i32, ptr @{{.*}}1q
//CHECK: call i32 @{{.*}}computeB
//CHECK: store i32 {{.*}}, ptr @{{.*}}1b

static int a = computeA();
static int q = a + 1;
static int b = q + computeB();

int computeA()
{
    return 3;
}

int computeB()
{
    return 4;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main() {}
