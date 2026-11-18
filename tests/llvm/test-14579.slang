//TEST:SIMPLE(filecheck=CHECK):-target llvm-host-ir -emit-cpu-via-llvm -whole-program

// Confirm a zero-parameter compute kernel legalizes to the CPU ABI's
// three context-pointer signature, matching the hardcoded calls in the
// emitted dispatcher and workgroup wrappers.
[shader("compute")]
[numthreads(1, 1, 1)]
void main() {}
//CHECK: define void @_S{{.*}}(ptr {{.*}}, ptr {{.*}}, ptr {{.*}})
//CHECK: define void @main_Group(ptr
//CHECK: define void @main(ptr
