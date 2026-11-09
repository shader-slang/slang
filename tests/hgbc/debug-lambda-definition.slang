//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main -g1

[shader("compute")]
[numthreads(1, 1, 1)]
void probe<T>() {}

[host]
void forward<T>(T value) { dispatch(probe<T>, uint3(1, 1, 1))(); }

[host]
void main()
{
    forward(() => 42u);
}

// The runtime closure is eliminated, but its canonical reflection name and definition remain.
// CHECK-DAG: "main._slang_Lambda_main_0"
// CHECK-DAG: OpDebugLambda {{.*}} 13 16
