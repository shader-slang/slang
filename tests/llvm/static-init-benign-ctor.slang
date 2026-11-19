//TEST:SIMPLE(filecheck=CHECK): -target llvm-host-ir -emit-cpu-via-llvm -whole-program -O0 -entry main -stage compute

// Regression for shader-slang/slang#14578: a module-scope static with a
// side-effecting initializer that no entry point references was silently
// dropped by the whole-program link on host targets, where statics are
// process-lifetime globals — the initializer's observable effect never ran.
// The link now preserves such globals; the existing defer-to-constructor
// pipeline emits them into __slang_global_constructor registered with
// llvm.global_ctors, so the initialization runs exactly once at load time.
// At higher optimization levels LLVM's GlobalOpt may soundly remove a
// provably-dead initializer (a pure call storing into a never-read global),
// so this test pins the emitted shape at -O0.

//CHECK-DAG: define internal void @__slang_global_constructor
//CHECK-DAG: call i32 @{{.*}}computeInitialValue
// The mangled name of `s` is `_SVR...1s`.
//CHECK-DAG: store i32 {{.*}}, ptr @{{.*}}1s
// The ctor is registered in llvm.global_ctors so it runs once at load.
//CHECK-DAG: llvm.global_ctors{{.*}}__slang_global_constructor

static int s = computeInitialValue();

int computeInitialValue()
{
    int sum = 0;
    for (int i = 0; i < 7; ++i)
        sum += i;
    return sum;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main() {}
