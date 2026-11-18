//TEST:SIMPLE(filecheck=CHECK): -target llvm-host-ir -emit-cpu-via-llvm -whole-program -O0 -entry main -stage compute

// Regression for shader-slang/slang#14578: the link now preserves host-target
// globals whose initializers might have side effects. This variant locks the
// dependency the plan flagged as a risk: the preserved global `b` has a
// side-effecting initializer that also READS the preserved global `a`, so
// cloning `b` must carry `a` along, and the module constructor must initialize
// `a` before the initializer of `b` loads it. Pinned at -O0 like
// static-init-benign-ctor.slang: at higher optimization levels LLVM may
// soundly fold provably-dead initializers away.

//CHECK-DAG: llvm.global_ctors{{.*}}__slang_global_constructor
//CHECK: define internal void @__slang_global_constructor
//CHECK: call i32 @{{.*}}computeA
//CHECK: store i32 {{.*}}, ptr @{{.*}}1a
//CHECK: load i32, ptr @{{.*}}1a
//CHECK: call i32 @{{.*}}computeB
//CHECK: store i32 {{.*}}, ptr @{{.*}}1b

static int a = computeA();
static int b = a + computeB();

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
